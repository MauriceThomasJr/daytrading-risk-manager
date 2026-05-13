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
    containerRef.current.innerHTML = ""

    // The widget script looks for a sibling div with this exact class.
    const widgetDiv = document.createElement("div")
    widgetDiv.className = "tradingview-widget-container__widget"
    widgetDiv.style.height = "100%"
    widgetDiv.style.width = "100%"
    containerRef.current.appendChild(widgetDiv)

    // Delay script injection by one tick so the container's height is fully
    // applied to the DOM before TradingView measures it.
    const timeoutId = setTimeout(() => {
      if (!containerRef.current) return

      const script = document.createElement("script")
      script.src =
        "https://s3.tradingview.com/external-embedding/embed-widget-advanced-chart.js"
      script.type = "text/javascript"
      script.async = true
      script.innerHTML = JSON.stringify({
        width: "100%",
        height: height,    // pixel number, not percentage string
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
    }, 0)

    return () => clearTimeout(timeoutId)
  }, [symbol, interval])

  return (
    <div
      className="tradingview-widget-container"
      ref={containerRef}
      style={{ height: `${height}px`, width: "100%" }}
    />
  )
}