export type Side = "Long" | "Short"

export interface Instrument {
  symbol: string
  dollar_per_point: number
  tick_size: number
}

export interface TradeFormState {
  instrument: Instrument
  side: Side
  entry_price: number | ""
  stop_price: number | ""
  target_price: number | ""
}

export type ChecklistResponses = Record<string, boolean>

export interface ChecklistItem {
  id: string
  prompt: string
}