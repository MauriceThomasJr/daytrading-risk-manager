import { useState } from "react"
import { AccountPanel } from "@/components/AccountPanel"
import { ChecklistPanel } from "@/components/ChecklistPanel"
import { SubmitBar } from "@/components/SubmitBar"
import { TradeForm } from "@/components/TradeForm"

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

function App() {
  const [accountId, setAccountId] = useState("alice")
  const [templateId, setTemplateId] = useState("daily-es")
  const [responses, setResponses] = useState<ChecklistResponses>({})
  const [trade, setTrade] = useState<TradeFormState>(DEFAULT_TRADE)

  return (
    <div className="min-h-screen bg-gray-50 p-8">
      <div className="mx-auto max-w-3xl space-y-4">
        <div>
          <h1 className="text-2xl font-bold text-gray-900">
            Daytrading Risk Manager
          </h1>
          <p className="mt-1 text-sm text-gray-600">
            Pre-trade discipline gate over the local API.
          </p>
        </div>

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
      </div>
    </div>
  )
}

export default App