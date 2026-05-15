import { useEffect, useRef } from "react"
import {
  createChart,
  CandlestickSeries,
  LineStyle,
  type IChartApi,
  type ISeriesApi,
  type IPriceLine,
} from "lightweight-charts"
import type { OhlcvBar } from "@/types/chart"

interface LightweightChartProps {
  historicalBars: OhlcvBar[]
  latestBar?: OhlcvBar | null
  height?: number
  entryPrice?: number | null
  stopPrice?: number | null
  targetPrice?: number | null
}

export function LightweightChart({
  historicalBars,
  latestBar,
  height = 720,
  entryPrice,
  stopPrice,
  targetPrice,
}: LightweightChartProps) {
  const containerRef = useRef<HTMLDivElement>(null)
  const chartRef = useRef<IChartApi | null>(null)
  const seriesRef = useRef<ISeriesApi<"Candlestick"> | null>(null)
  const priceLinesRef = useRef<IPriceLine[]>([])

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
        rightOffset: 5,
        barSpacing: 8,
      },
      rightPriceScale: {
        borderColor: "#e0e0e0",
      },
      crosshair: {
        mode: 1,
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

  // When the historical bars change wholesale (timeframe switch, scrub
  // backward, new session), reset the entire series.
  useEffect(() => {
    if (!seriesRef.current) return

    seriesRef.current.setData(
      historicalBars.map((b) => ({
        time: b.time as never,
        open: b.open,
        high: b.high,
        low: b.low,
        close: b.close,
      }))
    )
  }, [historicalBars])

  // When a new latest bar arrives, push it incrementally. This is much
  // smoother than rebuilding the whole series each tick.
  useEffect(() => {
    if (!seriesRef.current || !latestBar) return

    seriesRef.current.update({
      time: latestBar.time as never,
      open: latestBar.open,
      high: latestBar.high,
      low: latestBar.low,
      close: latestBar.close,
    })
  }, [latestBar])

  // When trade levels change, redraw the price lines.
  useEffect(() => {
    const series = seriesRef.current
    if (!series) return

    for (const line of priceLinesRef.current) {
      series.removePriceLine(line)
    }
    priceLinesRef.current = []

    if (typeof entryPrice === "number" && !Number.isNaN(entryPrice)) {
      priceLinesRef.current.push(
        series.createPriceLine({
          price: entryPrice,
          color: "#2196f3",
          lineWidth: 2,
          lineStyle: LineStyle.Solid,
          axisLabelVisible: true,
          title: "Entry",
        })
      )
    }
    if (typeof stopPrice === "number" && !Number.isNaN(stopPrice)) {
      priceLinesRef.current.push(
        series.createPriceLine({
          price: stopPrice,
          color: "#ef5350",
          lineWidth: 2,
          lineStyle: LineStyle.Dashed,
          axisLabelVisible: true,
          title: "Stop",
        })
      )
    }
    if (typeof targetPrice === "number" && !Number.isNaN(targetPrice)) {
      priceLinesRef.current.push(
        series.createPriceLine({
          price: targetPrice,
          color: "#26a69a",
          lineWidth: 2,
          lineStyle: LineStyle.Dashed,
          axisLabelVisible: true,
          title: "Target",
        })
      )
    }
  }, [entryPrice, stopPrice, targetPrice])

  return (
    <div
      ref={containerRef}
      style={{ width: "100%", height: `${height}px` }}
    />
  )
}