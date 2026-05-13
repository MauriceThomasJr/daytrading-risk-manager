import { useEffect, useRef } from "react"

interface TradingViewChartProps {
  symbol: string
  interval?: string
  height?: number
}

export function TradingViewChart({
  symbol,
  interval = "5",
  height = 500,
}: TradingViewChartProps) {
  const containerRef = useRef<HTMLDivElement>(null)

  useEffect(() => {
    if (!containerRef.current) return

    // Clear any previous widget when symbol changes.
    containerRef.current.innerHTML =
      '<div class="tradingview-widget-container__widget" style="height: 100%; width: 100%;"></div>'

    const script = document.createElement("script")
    script.src =
      "https://s3.tradingview.com/external-embedding/embed-widget-advanced-chart.js"
    script.type = "text/javascript"
    script.async = true
    script.innerHTML = JSON.stringify({
      autosize: true,
      symbol,
      interval,
      timezone: "America/New_York",
      theme: "light",
      style: "1",
      locale: "en",
      allow_symbol_change: true,
      withdateranges: true,
      hide_side_toolbar: false,
      details: false,
      hotlist: false,
      calendar: false,
      support_host: "https://www.tradingview.com",
    })

    containerRef.current.appendChild(script)
  }, [symbol, interval])

  return (
    <div className="rounded-lg border border-gray-200 bg-white overflow-hidden">
      <div
        className="tradingview-widget-container"
        ref={containerRef}
        style={{ height: `${height}px`, width: "100%" }}
      />
    </div>
  )
}