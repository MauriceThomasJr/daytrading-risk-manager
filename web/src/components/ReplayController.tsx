import { Button } from "@/components/ui/button"
import type { Timeframe } from "@/lib/aggregation"

interface ReplayControllerProps {
  timeframe: Timeframe
  onTimeframeChange: (tf: Timeframe) => void
  isPlaying: boolean
  onPlayPauseToggle: () => void
  speed: number
  onSpeedChange: (speed: number) => void
  currentTime: number
  sessionStart: number
  sessionEnd: number
  fractionElapsed: number
  onSeekFraction: (fraction: number) => void
}

const TIMEFRAMES: Timeframe[] = ["1m", "5m", "15m", "30m", "1h"]
const SPEEDS = [
  { label: "1x", value: 1 },
  { label: "4x", value: 4 },
  { label: "16x", value: 16 },
  { label: "Fastest", value: 64 },
]

export function ReplayController({
  timeframe,
  onTimeframeChange,
  isPlaying,
  onPlayPauseToggle,
  speed,
  onSpeedChange,
  currentTime,
  sessionStart,
  sessionEnd,
  fractionElapsed,
  onSeekFraction,
}: ReplayControllerProps) {
  const atEnd = currentTime >= sessionEnd

  return (
    <div className="mt-2 rounded-lg border border-gray-200 bg-white p-3">
      <div className="flex flex-wrap items-center gap-3">
        <Button size="sm" onClick={onPlayPauseToggle} disabled={atEnd}>
          {isPlaying ? "Pause" : "Play"}
        </Button>

        <div className="flex items-center gap-1">
          <span className="text-xs text-gray-600 mr-1">Speed:</span>
          {SPEEDS.map((s) => (
            <Button
              key={s.value}
              size="sm"
              variant={speed === s.value ? "default" : "outline"}
              onClick={() => onSpeedChange(s.value)}
            >
              {s.label}
            </Button>
          ))}
        </div>

        <div className="flex items-center gap-1">
          <span className="text-xs text-gray-600 mr-1">Timeframe:</span>
          {TIMEFRAMES.map((tf) => (
            <Button
              key={tf}
              size="sm"
              variant={timeframe === tf ? "default" : "outline"}
              onClick={() => onTimeframeChange(tf)}
            >
              {tf}
            </Button>
          ))}
        </div>

        <div className="text-xs text-gray-600 ml-auto font-mono">
          {formatTime(currentTime)} / {formatTime(sessionEnd)}
        </div>
      </div>

      <div className="mt-3">
        <input
          type="range"
          min={0}
          max={1000}
          value={Math.round(fractionElapsed * 1000)}
          onChange={(e) => onSeekFraction(parseInt(e.target.value, 10) / 1000)}
          className="w-full"
        />
        <div className="mt-1 text-xs text-gray-500">
          {(fractionElapsed * 100).toFixed(1)}% through session
          {" · "}
          {formatDuration(currentTime - sessionStart)} elapsed
        </div>
      </div>
    </div>
  )
}

function formatTime(unixSeconds: number): string {
  const d = new Date(unixSeconds * 1000)
  return d.toLocaleTimeString("en-US", {
    hour: "2-digit",
    minute: "2-digit",
    hour12: false,
  })
}

function formatDuration(seconds: number): string {
  const totalMinutes = Math.floor(seconds / 60)
  const hours = Math.floor(totalMinutes / 60)
  const mins = totalMinutes % 60
  if (hours > 0) return `${hours}h ${mins}m`
  return `${mins}m`
}