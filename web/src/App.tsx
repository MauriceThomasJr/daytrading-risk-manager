import { useState } from "react"
import { useQuery } from "@tanstack/react-query"
import { AccountPanel } from "@/components/AccountPanel"
import { ChecklistPanel } from "@/components/ChecklistPanel"
import { SubmitBar } from "@/components/SubmitBar"
import { TradeForm } from "@/components/TradeForm"
import { RecentTrades } from "@/components/RecentTrades"
import { LightweightChart } from "@/components/LightweightChart"
import { fetchHistoricalBars } from "@/lib/api"

import type {
  TradeFormState,
  ChecklistResponses,
  ChecklistItem,
} from "@/types/trade"

const DEFAULT_CHECKLIST_ITEMS: ChecklistItem[] = [
  { id: "volume", prompt: "Is there volume confirmation?" },
  { id: "htf", prompt: "Did you check the higher timeframe?" },
  { id: "plan", prompt: "Is this trade in your written plan?" },
]

const DEFAULT_TRADE: TradeFormState = {
  instrument: { symbol: "MES", dollar_per_point: 5, tick_size: 0.25 },
  side: "Long",
  entry_price: 7000,
  stop_price: 6991,
  target_price: 7050,
}


const SESSION_DATE = "2026-02-13"  //

function App() {
  const [accountId, setAccountId] = useState("alice")
  const [templateId, setTemplateId] = useState("daily-es")
  const [responses, setResponses] = useState<ChecklistResponses>({})
  const [trade, setTrade] = useState<TradeFormState>(DEFAULT_TRADE)

  const barsQuery = useQuery({
    queryKey: ["bars", "nq", SESSION_DATE],
    queryFn: () => fetchHistoricalBars("nq", SESSION_DATE),
  })

  return (
    <div className="min-h-screen bg-gray-50">
      <div className="border-b border-gray-200 bg-white px-6 py-3">
        <h1 className="text-lg font-bold text-gray-900">
          Daytrading Risk Manager
        </h1>
        <p className="text-xs text-gray-600">
          Pre-trade discipline gate over the local API.
        </p>
      </div>

      <div className="flex flex-col lg:flex-row gap-4 p-4">
        {/* Chart column — dominates on wide screens */}
        <div className="lg:flex-1 lg:min-w-0">
          <div className="rounded-lg border border-gray-200 bg-white overflow-hidden">
            {barsQuery.isLoading && (
              <div className="h-[720px] flex items-center justify-center text-gray-500">
                Loading chart data...
              </div>
            )}
            {barsQuery.isError && (
              <div className="h-[720px] flex items-center justify-center text-red-600">
                {(barsQuery.error as Error).message}
              </div>
            )}
            {barsQuery.isSuccess && (
              <LightweightChart bars={barsQuery.data} height={720} />
            )}
          </div>
          <p className="mt-1 px-1 text-xs text-gray-500">
            NQ — {SESSION_DATE}
          </p>
        </div>

        {/* Control sidebar — fixed width on wide screens */}
        <div className="lg:w-96 lg:flex-shrink-0 space-y-3">
          <AccountPanel
            accountId={accountId}
            onAccountIdChange={setAccountId}
          />

          <ChecklistPanel
            templateId={templateId}
            onTemplateIdChange={setTemplateId}
            items={DEFAULT_CHECKLIST_ITEMS}
            responses={responses}
            onResponsesChange={setResponses}
          />

          <TradeForm state={trade} onChange={setTrade} />

          <SubmitBar
            accountId={accountId}
            templateId={templateId}
            trade={trade}
            responses={responses}
          />

          <RecentTrades />
        </div>
      </div>
    </div>
  )
}

export default App