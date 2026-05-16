export type Side = "Long" | "Short"

export interface Instrument {
  symbol: string
  dollar_per_point: number
  tick_size: number
}

export interface TradeFormState {
  instrument: Instrument
  side: "Long" | "Short"
  size: number | ""
  stop_price: number | ""
  target_price: number | ""
}

export type ChecklistResponses = Record<string, boolean>

export interface ChecklistItem {
  id: string
  prompt: string
}
export interface TradeRequest {
  account_id: string
  template_id: string
  side: "Long" | "Short"
  current_price: number
  size: number
  stop_price: number
  target_price?: number
  instrument: Instrument
  checklist_responses: ChecklistResponses
}

export interface OrderResponse {
  id: number
  side: Side
  size: number
  symbol: string
  entry_price: number
  stop_price: number
  target_price?: number
}

export interface TradeResult {
  accepted: boolean
  order?: OrderResponse
  rejection_reasons?: string[]
  error?: string
  account_id?: string
  template_id?: string
}
export interface TradesListResponse {
  trades: OrderResponse[]
}