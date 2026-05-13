import { useEffect, useRef } from "react"
import { createChart, CandlestickSeries, type IChartApi, type ISeriesApi } from "lightweight-charts"
import type { OhlcvBar } from "@/types/chart"

interface LightweightChartProps {
  bars: OhlcvBar[]
  height?: number
}

export function LightweightChart({ bars, height = 720 }: LightweightChartProps) {
  const containerRef = useRef<HTMLDivElement>(null)
  const chartRef = useRef<IChartApi | null>(null)
  const seriesRef = useRef<ISeriesApi<"Candlestick"> | null>(null)

  // Create the chart once on mount. Tear down on unmount.
  useEffect(() => {
    if (!containerRef.current) return

    const chart = createChart(containerRef.current, {
      width: containerRef.current.clientWidth,
      height,
      layout: {
        background: { color: "#ffffff" },
        textColor: "#1a1a1a",
      },
      grid: {
        vertLines: { color: "#f0f0f0" },
        horzLines: { color: "#f0f0f0" },
      },
      timeScale: {
        timeVisible: true,
        secondsVisible: false,
        borderColor: "#e0e0e0",
      },
      rightPriceScale: {
        borderColor: "#e0e0e0",
      },
      crosshair: {
        mode: 1,  // Normal crosshair, follows mouse
      },
    })

    const series = chart.addSeries(CandlestickSeries, {
      upColor: "#26a69a",
      downColor: "#ef5350",
      borderUpColor: "#26a69a",
      borderDownColor: "#ef5350",
      wickUpColor: "#26a69a",
      wickDownColor: "#ef5350",
    })

    chartRef.current = chart
    seriesRef.current = series

    // Keep the chart responsive to window resizes.
    const handleResize = () => {
      if (containerRef.current && chartRef.current) {
        chartRef.current.applyOptions({
          width: containerRef.current.clientWidth,
        })
      }
    }
    window.addEventListener("resize", handleResize)

    return () => {
      window.removeEventListener("resize", handleResize)
      chart.remove()
    }
  }, [height])

  // When bars change, push them to the series.
  useEffect(() => {
    if (!seriesRef.current) return

    seriesRef.current.setData(
      bars.map((b) => ({
        time: b.time as never,  // Lightweight Charts has a strict Time type; cast for simplicity
        open: b.open,
        high: b.high,
        low: b.low,
        close: b.close,
      }))
    )

    // Auto-fit the view to show all bars.
    if (chartRef.current) {
      chartRef.current.timeScale().fitContent()
    }
  }, [bars])

  return (
    <div
      ref={containerRef}
      style={{ width: "100%", height: `${height}px` }}
    />
  )
}