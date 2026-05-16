import { useMutation, useQueryClient } from "@tanstack/react-query"
import { closeTrade } from "@/lib/api"
import { Button } from "@/components/ui/button"
import type { OrderResponse } from "@/types/trade"

interface ActiveTradePanelProps {
  activeOrder: OrderResponse | null
  accountId: string
  currentPrice: number | null
  onTradeClosed: () => void
}

export function ActiveTradePanel({
  activeOrder,
  accountId,
  currentPrice,
  onTradeClosed,
}: ActiveTradePanelProps) {
  const queryClient = useQueryClient()

  const closeMutation = useMutation({
    mutationFn: (exitPrice: number) =>
      closeTrade(activeOrder!.id, {
        account_id: accountId,
        exit_price: exitPrice,
      }),
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ["account", accountId] })
      queryClient.invalidateQueries({ queryKey: ["recentTrades"] })
      onTradeClosed()
    },
  })

  // Empty state.
  if (!activeOrder) {
    return (
      <div className="rounded-lg border border-gray-200 bg-white p-5">
        <h2 className="mb-3 text-xs font-semibold uppercase tracking-wider text-gray-600">
          Active Trade
        </h2>
        <div className="text-sm text-gray-500 italic">
          No active trade. Submit one to begin.
        </div>
      </div>
    )
  }

  // Compute unrealized P&L from the chart's current price.
  let unrealizedPnL: number | null = null
  if (currentPrice !== null) {
    const pricediff =
      activeOrder.side === "Long"
        ? currentPrice - activeOrder.entry_price
        : activeOrder.entry_price - currentPrice
    // We don't have $/point in the order response here. The frontend types
    // file only carries it on the form. For now we just show the price
    // delta — the user can multiply by their contract value in their head.
    unrealizedPnL = pricediff * activeOrder.size
  }

  function handleClose() {
    if (currentPrice === null) {
      alert("Chart not ready. Cannot close.")
      return
    }
    closeMutation.mutate(currentPrice)
  }

  const sideColor =
    activeOrder.side === "Long" ? "text-green-700" : "text-red-700"

  const pnlColor =
    unrealizedPnL === null
      ? "text-gray-700"
      : unrealizedPnL >= 0
      ? "text-green-700"
      : "text-red-700"

  return (
    <div className="rounded-lg border border-blue-200 bg-blue-50 p-5">
      <h2 className="mb-3 text-xs font-semibold uppercase tracking-wider text-blue-900">
        Active Trade
      </h2>

      <div className="mb-3 flex items-baseline gap-2">
        <span className="text-xs text-gray-600">Order</span>
        <span className="text-sm font-mono">#{activeOrder.id}</span>
        <span className={`text-sm font-semibold ${sideColor}`}>
          {activeOrder.side}
        </span>
        <span className="text-sm">{activeOrder.symbol}</span>
        <span className="text-xs text-gray-600 ml-auto">
          {activeOrder.size} contracts
        </span>
      </div>

      <div className="space-y-1.5 mb-4 font-mono text-sm">
        <PriceRow label="Entry" value={activeOrder.entry_price} color="text-blue-700" />
        <PriceRow label="Stop" value={activeOrder.stop_price} color="text-red-700" />
        {activeOrder.target_price !== undefined && (
          <PriceRow label="Target" value={activeOrder.target_price} color="text-green-700" />
        )}
        {currentPrice !== null && (
          <PriceRow label="Current" value={currentPrice} color="text-gray-700" />
        )}
      </div>

      {unrealizedPnL !== null && (
        <div className="mb-3 text-sm">
          <span className="text-gray-600">Price delta (× size): </span>
          <span className={`font-mono font-semibold ${pnlColor}`}>
            {unrealizedPnL >= 0 ? "+" : ""}
            {unrealizedPnL.toFixed(2)}
          </span>
        </div>
      )}

      <div className="border-t border-blue-200 pt-3">
        <div className="flex items-center gap-3">
          {currentPrice !== null && (
            <span className="text-xs text-gray-600">
              Close at{" "}
              <span className="font-mono font-semibold text-gray-900">
                {currentPrice.toFixed(2)}
              </span>
            </span>
          )}
          <Button
            size="sm"
            onClick={handleClose}
            disabled={closeMutation.isPending || currentPrice === null}
            className="ml-auto"
          >
            {closeMutation.isPending ? "Closing..." : "Close at Market"}
          </Button>
        </div>
        {closeMutation.isError && (
          <div className="mt-2 text-xs text-red-600">
            {(closeMutation.error as Error).message}
          </div>
        )}
      </div>
    </div>
  )
}

function PriceRow({
  label,
  value,
  color,
}: {
  label: string
  value: number
  color: string
}) {
  return (
    <div className="flex justify-between">
      <span className="text-gray-700">{label}</span>
      <span className={`font-semibold ${color}`}>{value.toFixed(2)}</span>
    </div>
  )
}