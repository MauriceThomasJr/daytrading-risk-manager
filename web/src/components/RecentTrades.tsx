import { useQuery } from "@tanstack/react-query"
import { fetchRecentTrades } from "@/lib/api"
import { Button } from "@/components/ui/button"
import type { OrderResponse } from "@/types/trade"

export function RecentTrades() {
  const query = useQuery({
    queryKey: ["recentTrades"],
    queryFn: () => fetchRecentTrades(10),
  })

  return (
    <div className="rounded-lg border border-gray-200 bg-white p-5">
      <div className="flex items-center justify-between mb-3">
        <h2 className="text-xs font-semibold uppercase tracking-wider text-gray-600">
          Recent Trades
        </h2>
        <Button
          variant="outline"
          size="sm"
          onClick={() => query.refetch()}
          disabled={query.isFetching}
        >
          {query.isFetching ? "Refreshing..." : "Refresh"}
        </Button>
      </div>

      <div className="min-h-[80px]">
        {query.isLoading && (
          <div className="text-sm text-gray-500">Loading...</div>
        )}

        {query.isError && (
          <div className="text-sm text-red-600">
            Failed to load trades: {(query.error as Error).message}
          </div>
        )}

        {query.isSuccess && <TradeList trades={query.data.trades} />}
      </div>
    </div>
  )
}

function TradeList({ trades }: { trades: OrderResponse[] }) {
  if (trades.length === 0) {
    return (
      <div className="text-sm text-gray-500 italic">No trades yet.</div>
    )
  }

  return (
    <div className="divide-y divide-gray-100">
      {trades.map((trade) => (
        <TradeRow key={trade.id} trade={trade} />
      ))}
    </div>
  )
}

function TradeRow({ trade }: { trade: OrderResponse }) {
  const sideColor = trade.side === "Long" ? "text-green-700" : "text-red-700"

  return (
    <div className="flex gap-3 py-2 text-sm font-mono">
      <span className="text-gray-500 min-w-[40px]">#{trade.id}</span>
      <span className={`min-w-[50px] font-semibold ${sideColor}`}>
        {trade.side}
      </span>
      <span className="min-w-[40px]">{trade.symbol}</span>
      <span className="flex-1 text-gray-700">
        {trade.size} @ {trade.entry_price}
      </span>
      <span className="text-gray-500">stop {trade.stop_price}</span>
    </div>
  )
}