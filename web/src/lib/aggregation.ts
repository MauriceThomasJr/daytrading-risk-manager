import type { OhlcvBar } from "@/types/chart"

export type Timeframe = "1m" | "5m" | "15m" | "30m" | "1h"

const TIMEFRAME_MINUTES: Record<Timeframe, number> = {
  "1m": 1,
  "5m": 5,
  "15m": 15,
  "30m": 30,
  "1h": 60,
}

export function alignToTimeframe(
  timeSeconds: number,
  timeframe: Timeframe,
): number {
  const minutes = TIMEFRAME_MINUTES[timeframe]
  const windowSeconds = minutes * 60
  return Math.floor(timeSeconds / windowSeconds) * windowSeconds
}

export function aggregateBars(
  bars: OhlcvBar[],
  timeframe: Timeframe,
): OhlcvBar[] {
  if (timeframe === "1m" || bars.length === 0) {
    return bars
  }

  const aggregated: OhlcvBar[] = []
  let currentWindowStart: number | null = null
  let currentGroup: OhlcvBar[] = []

  for (const bar of bars) {
    const windowStart = alignToTimeframe(bar.time, timeframe)

    if (currentWindowStart === null) {
      currentWindowStart = windowStart
      currentGroup = [bar]
    } else if (windowStart === currentWindowStart) {
      currentGroup.push(bar)
    } else {
      aggregated.push(combineBars(currentGroup, currentWindowStart))
      currentWindowStart = windowStart
      currentGroup = [bar]
    }
  }

  if (currentGroup.length > 0 && currentWindowStart !== null) {
    aggregated.push(combineBars(currentGroup, currentWindowStart))
  }

  return aggregated
}

function combineBars(group: OhlcvBar[], windowStart: number): OhlcvBar {
  return {
    time: windowStart,
    open: group[0].open,
    high: Math.max(...group.map((b) => b.high)),
    low: Math.min(...group.map((b) => b.low)),
    close: group[group.length - 1].close,
    volume: group.reduce((sum, b) => sum + b.volume, 0),
  }
}