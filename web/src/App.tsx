import { useState } from "react"
import { AccountPanel } from "@/components/AccountPanel"
import { ChecklistPanel } from "@/components/ChecklistPanel"
import { SubmitBar } from "@/components/SubmitBar"
import { TradeForm } from "@/components/TradeForm"
import { RecentTrades } from "@/components/RecentTrades"
import { TradingViewChart } from "@/components/TradingViewChart"

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
function symbolToTradingView(symbol: string): string {
  // The free TradingView widget doesn't serve CME futures data to embeds.
  // We show stock ETF proxies that track each futures contract 1:1.
  const upper = symbol.toUpperCase().trim()
  const map: Record<string, string> = {
    ES:  "AMEX:SPY",     // SPY tracks ES (S&P 500)
    MES: "AMEX:SPY",
    NQ:  "NASDAQ:QQQ",   // QQQ tracks NQ (Nasdaq-100)
    MNQ: "NASDAQ:QQQ",
    RTY: "AMEX:IWM",     // IWM tracks RTY (Russell 2000)
    M2K: "AMEX:IWM",
    YM:  "AMEX:DIA",     // DIA tracks YM (Dow)
    MYM: "AMEX:DIA",
  }
  return map[upper] ?? upper  // Anything else (CL, GC, etc.) passes through
}
function getProxyNote(symbol: string): string {
  const upper = symbol.toUpperCase().trim()
  const notes: Record<string, string> = {
    ES:  "SPY (tracks ES 1:1)",
    MES: "SPY (tracks MES 1:1)",
    NQ:  "QQQ (tracks NQ 1:1)",
    MNQ: "QQQ (tracks MNQ 1:1)",
    RTY: "IWM (tracks RTY)",
    M2K: "IWM (tracks M2K)",
    YM:  "DIA (tracks YM)",
    MYM: "DIA (tracks MYM)",
  }
  return notes[upper] ?? upper
}
function App() {
  const [accountId, setAccountId] = useState("alice")
  const [templateId, setTemplateId] = useState("daily-es")
  const [responses, setResponses] = useState<ChecklistResponses>({})
  const [trade, setTrade] = useState<TradeFormState>(DEFAULT_TRADE)

  const tvSymbol = symbolToTradingView(trade.instrument.symbol)
  

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
      {/* Chart column — dominates on wide screens */}
      <div className="lg:flex-1 lg:min-w-0">
        <div className="rounded-lg border border-gray-200 bg-white overflow-hidden">
          <TradingViewChart symbol={tvSymbol} interval="5" height={720} />
        </div>
        <p className="mt-1 px-1 text-xs text-gray-500">
          Showing {getProxyNote(trade.instrument.symbol)}
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