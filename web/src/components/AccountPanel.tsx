import { useState } from "react"
import { useQuery } from "@tanstack/react-query"
import { fetchAccount } from "@/lib/api"
import { Button } from "@/components/ui/button"

export function AccountPanel() {
  const [accountId, setAccountId] = useState("alice")
  const [loadedId, setLoadedId] = useState<string | null>(null)

  const query = useQuery({
    queryKey: ["account", loadedId],
    queryFn: () => fetchAccount(loadedId!),
    enabled: loadedId !== null,
  })

  return (
    <div className="rounded-lg border border-gray-200 bg-white p-5">
      <h2 className="mb-3 text-xs font-semibold uppercase tracking-wider text-gray-600">
        Account
      </h2>

      <div className="flex gap-2 items-center">
        <label htmlFor="account-id" className="w-32 text-sm text-gray-600">
          Account ID
        </label>
        <input
          id="account-id"
          type="text"
          value={accountId}
          onChange={(e) => setAccountId(e.target.value)}
          className="flex-1 rounded border border-gray-300 px-2 py-1 text-sm"
        />
        <Button
          size="sm"
          onClick={() => setLoadedId(accountId.trim())}
        >
          Load
        </Button>
      </div>

      <div className="mt-4 min-h-[80px]">
        {loadedId === null && (
          <div className="text-sm text-gray-500 italic">
            Enter an account ID and click Load.
          </div>
        )}

        {query.isLoading && (
          <div className="text-sm text-gray-500">Loading...</div>
        )}

        {query.isError && (
          <div className="text-sm text-red-600">
            {(query.error as Error).message}
          </div>
        )}

        {query.isSuccess && (
          <div className="space-y-1 text-sm">
            <Stat label="Balance" value={formatMoney(query.data.balance)} />
            <Stat label="Daily P&L" value={formatMoney(query.data.daily_pnl)} />
            <Stat label="Trades today" value={String(query.data.trades_today)} />
          </div>
        )}
      </div>
    </div>
  )
}

function Stat({ label, value }: { label: string; value: string }) {
  return (
    <div className="flex justify-between font-mono">
      <span className="text-gray-600">{label}</span>
      <span className="text-gray-900 font-medium">{value}</span>
    </div>
  )
}

function formatMoney(n: number): string {
  return new Intl.NumberFormat("en-US", {
    style: "currency",
    currency: "USD",
  }).format(n)
}