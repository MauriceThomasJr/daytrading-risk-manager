import { useState, useMemo, useEffect } from "react"
import { useQuery } from "@tanstack/react-query"
import { AccountPanel } from "@/components/AccountPanel"
import { ChecklistPanel } from "@/components/ChecklistPanel"
import { SubmitBar } from "@/components/SubmitBar"
import { TradeForm } from "@/components/TradeForm"
import { RecentTrades } from "@/components/RecentTrades"
import { LightweightChart } from "@/components/LightweightChart"
import { ReplayController } from "@/components/ReplayController"
import { fetchHistoricalBars } from "@/lib/api"
import { aggregateBars, alignToTimeframe, type Timeframe } from "@/lib/aggregation"

import type {
  TradeFormState,
  ChecklistResponses,
  ChecklistItem,
} from "@/types/trade"

const DEFAULT_CHECKLIST_ITEMS: ChecklistItem[] = [
  { id: "volume", prompt: "Is there volume confirmation?" },
  { id: "htf", prompt: "Did you check the higher timeframe?" },
  { id: "plan", prompt: "Is this trade in your written plan?" },
]

const DEFAULT_TRADE: TradeFormState = {
  instrument: { symbol: "NQ", dollar_per_point: 20, tick_size: 0.25 },
  side: "Long",
  entry_price: 24773,
  stop_price: 24730,
  target_price: 25000,
}

const SESSION_DATE = "2026-02-13"

// Always advance playback at the 1-minute rate. Each tick = 60 seconds of
// market time. Timeframe only affects how the data is presented, not how
// quickly time passes.
const SECONDS_PER_TICK = 60

function App() {
  const [accountId, setAccountId] = useState("alice")
  const [templateId, setTemplateId] = useState("daily-es")
  const [responses, setResponses] = useState<ChecklistResponses>({})
  const [trade, setTrade] = useState<TradeFormState>(DEFAULT_TRADE)
  const [timeframe, setTimeframe] = useState<Timeframe>("1m")
  const [isPlaying, setIsPlaying] = useState(false)
  const [speed, setSpeed] = useState(1)

  // Source of truth: the current wall-clock time of the playback cursor,
  // in Unix seconds. Everything else is derived from this.
  const [currentTime, setCurrentTime] = useState<number | null>(null)

  const barsQuery = useQuery({
    queryKey: ["bars", "nq", SESSION_DATE],
    queryFn: () => fetchHistoricalBars("nq", SESSION_DATE),
  })

  // Initialize the cursor to 30 minutes into the session once data loads.
  useEffect(() => {
    if (barsQuery.data && currentTime === null) {
      const sessionStart = barsQuery.data[0].time
      const initialOffset = 30 * 60  // 30 minutes
      setCurrentTime(sessionStart + initialOffset)
    }
  }, [barsQuery.data, currentTime])

  // Aggregated bars for the chosen timeframe.
  const aggregatedBars = useMemo(() => {
    if (!barsQuery.data) return []
    return aggregateBars(barsQuery.data, timeframe)
  }, [barsQuery.data, timeframe])

  // Find the index of the bar whose window contains currentTime.
  // The bar at this index is the "current" one — partially formed if
  // currentTime is mid-window, fully formed if currentTime is past it.
  const currentAggregatedIndex = useMemo(() => {
    if (currentTime === null || aggregatedBars.length === 0) return -1

    const windowSeconds = getTimeframeSeconds(timeframe)
    // Find the bar whose [start, start+window) range contains currentTime.
    for (let i = 0; i < aggregatedBars.length; i++) {
      if (aggregatedBars[i].time + windowSeconds > currentTime) {
        return i
      }
    }
    return aggregatedBars.length - 1
  }, [currentTime, aggregatedBars, timeframe])

  // All bars before the currently-forming one. These are fully closed.
  const historicalBars = useMemo(() => {
    if (currentAggregatedIndex <= 0) return []
    return aggregatedBars.slice(0, currentAggregatedIndex)
  }, [aggregatedBars, currentAggregatedIndex])

  // The currently-forming (or just-completed) bar. As currentTime advances
  // within a window, this bar's high/low/close update.
  const latestBar = useMemo(() => {
    if (currentAggregatedIndex < 0 || currentTime === null) return null
    if (!barsQuery.data) return null

    const windowSeconds = getTimeframeSeconds(timeframe)
    const containingBar = aggregatedBars[currentAggregatedIndex]
    const windowStart = containingBar.time
    const windowEnd = windowStart + windowSeconds

    // Recompute the current bar from the underlying 1m bars that fall
    // within this window and are at or before currentTime. This gives the
    // partial-bar effect: as time advances, the bar's high/low/close shift.
    const contributingBars = barsQuery.data.filter(
      (b) => b.time >= windowStart && b.time < windowEnd && b.time <= currentTime
    )

    if (contributingBars.length === 0) return null

    return {
      time: windowStart,
      open: contributingBars[0].open,
      high: Math.max(...contributingBars.map((b) => b.high)),
      low: Math.min(...contributingBars.map((b) => b.low)),
      close: contributingBars[contributingBars.length - 1].close,
      volume: contributingBars.reduce((s, b) => s + b.volume, 0),
    }
  }, [aggregatedBars, currentAggregatedIndex, currentTime, barsQuery.data, timeframe])

  // Session boundaries for the progress display.
  const sessionStart = barsQuery.data?.[0]?.time ?? null
  const sessionEnd = barsQuery.data?.[barsQuery.data.length - 1]?.time ?? null

  // Reset the chart when the timeframe changes. Pause and re-anchor to the
  // start of the bar that contains the current time.
  useEffect(() => {
    setIsPlaying(false)
    if (currentTime !== null) {
      // Snap currentTime to the start of its window in the new timeframe.
      // This keeps the cursor aligned cleanly when switching timeframes.
      setCurrentTime(alignToTimeframe(currentTime, timeframe))
    }
    // We intentionally only re-anchor on timeframe change.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [timeframe])

  // Playback interval — advances currentTime at the 1m rate, regardless
  // of the displayed timeframe. Each tick is SECONDS_PER_TICK of market time.
  useEffect(() => {
    if (!isPlaying || currentTime === null || sessionEnd === null) return

    const intervalMs = 1000 / speed

    const id = setInterval(() => {
      setCurrentTime((prev) => {
        if (prev === null) return prev
        const next = prev + SECONDS_PER_TICK
        if (next > sessionEnd) {
          setIsPlaying(false)
          return sessionEnd
        }
        return next
      })
    }, intervalMs)

    return () => clearInterval(id)
  }, [isPlaying, speed, currentTime, sessionEnd])

  // Allow the scrubber to set time directly. The controller passes back a
  // value in [0, 1] representing fraction of the session elapsed.
  function handleSeekFraction(fraction: number) {
    if (sessionStart === null || sessionEnd === null) return
    const sessionLength = sessionEnd - sessionStart
    setCurrentTime(sessionStart + sessionLength * fraction)
  }

  // Fraction elapsed for the controller's display.
  const fractionElapsed = useMemo(() => {
    if (currentTime === null || sessionStart === null || sessionEnd === null) return 0
    const sessionLength = sessionEnd - sessionStart
    if (sessionLength === 0) return 0
    return Math.max(0, Math.min(1, (currentTime - sessionStart) / sessionLength))
  }, [currentTime, sessionStart, sessionEnd])

  return (
    <div className="min-h-screen bg-gray-50">
      <div className="border-b border-gray-200 bg-white px-6 py-3">
        <h1 className="text-lg font-bold text-gray-900">
          Daytrading Risk Manager
        </h1>
        <p className="text-xs text-gray-600">
          Pre-trade discipline gate over the local API.
        </p>
      </div>

      <div className="flex flex-col lg:flex-row gap-4 p-4">
        <div className="lg:flex-1 lg:min-w-0">
          <div className="rounded-lg border border-gray-200 bg-white overflow-hidden">
            {barsQuery.isLoading && (
              <div className="h-[720px] flex items-center justify-center text-gray-500">
                Loading chart data...
              </div>
            )}
            {barsQuery.isError && (
              <div className="h-[720px] flex items-center justify-center text-red-600">
                {(barsQuery.error as Error).message}
              </div>
            )}
            {barsQuery.isSuccess && (
              <LightweightChart
                historicalBars={historicalBars}
                latestBar={latestBar}
                height={720}
                entryPrice={typeof trade.entry_price === "number" ? trade.entry_price : null}
                stopPrice={typeof trade.stop_price === "number" ? trade.stop_price : null}
                targetPrice={typeof trade.target_price === "number" ? trade.target_price : null}
              />
            )}
          </div>
          {barsQuery.isSuccess && currentTime !== null && (
            <ReplayController
              timeframe={timeframe}
              onTimeframeChange={setTimeframe}
              isPlaying={isPlaying}
              onPlayPauseToggle={() => setIsPlaying(!isPlaying)}
              speed={speed}
              onSpeedChange={setSpeed}
              currentTime={currentTime}
              sessionStart={sessionStart!}
              sessionEnd={sessionEnd!}
              fractionElapsed={fractionElapsed}
              onSeekFraction={handleSeekFraction}
            />
          )}
          <p className="mt-1 px-1 text-xs text-gray-500">
            NQ — {SESSION_DATE}
          </p>
        </div>

        <div className="lg:w-96 lg:flex-shrink-0 space-y-3">
          <AccountPanel
            accountId={accountId}
            onAccountIdChange={setAccountId}
          />

          <ChecklistPanel
            templateId={templateId}
            onTemplateIdChange={setTemplateId}
            items={DEFAULT_CHECKLIST_ITEMS}
            responses={responses}
            onResponsesChange={setResponses}
          />

          <TradeForm state={trade} onChange={setTrade} />

          <SubmitBar
            accountId={accountId}
            templateId={templateId}
            trade={trade}
            responses={responses}
          />

          <RecentTrades />
        </div>
      </div>
    </div>
  )
}

function getTimeframeSeconds(tf: Timeframe): number {
  const map = { "1m": 60, "5m": 300, "15m": 900, "30m": 1800, "1h": 3600 }
  return map[tf]
}

export default App