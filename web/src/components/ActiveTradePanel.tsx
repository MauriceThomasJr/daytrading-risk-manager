import { useState } from "react"
import { useMutation, useQueryClient } from "@tanstack/react-query"
import { closeTrade } from "@/lib/api"
import { Button } from "@/components/ui/button"
import type { OrderResponse } from "@/types/trade"

interface ActiveTradePanelProps {
  activeOrder: OrderResponse | null
  accountId: string
  onTradeClosed: () => void
}

export function ActiveTradePanel({
  activeOrder,
  accountId,
  onTradeClosed,
}: ActiveTradePanelProps) {
  const queryClient = useQueryClient()
  const [exitPriceInput, setExitPriceInput] = useState<string>("")

  const closeMutation = useMutation({
    mutationFn: (exitPrice: number) =>
      closeTrade(activeOrder!.id, {
        account_id: accountId,
        exit_price: exitPrice,
      }),
    onSuccess: () => {
      queryClient.invalidateQueries({ queryKey: ["account", accountId] })
      queryClient.invalidateQueries({ queryKey: ["recentTrades"] })
      setExitPriceInput("")
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

  function handleClose() {
    const price = parseFloat(exitPriceInput)
    if (Number.isNaN(price)) {
      alert("Enter a valid exit price.")
      return
    }
    closeMutation.mutate(price)
  }

  const sideColor =
    activeOrder.side === "Long" ? "text-green-700" : "text-red-700"

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
      </div>

      <div className="border-t border-blue-200 pt-3">
        <div className="flex gap-2 items-center">
          <label htmlFor="exit-price" className="text-xs text-gray-600">
            Close at
          </label>
          <input
            id="exit-price"
            type="number"
            step="0.01"
            value={exitPriceInput}
            onChange={(e) => setExitPriceInput(e.target.value)}
            className="flex-1 rounded border border-gray-300 px-2 py-1 text-sm"
            placeholder="exit price"
          />
          <Button
            size="sm"
            onClick={handleClose}
            disabled={closeMutation.isPending}
          >
            {closeMutation.isPending ? "Closing..." : "Close"}
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