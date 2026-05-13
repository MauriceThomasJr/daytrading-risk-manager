import type { Account } from "@/types/account"

const API_BASE = "http://localhost:8080"

export async function fetchAccount(accountId: string): Promise<Account> {
  const res = await fetch(`${API_BASE}/accounts/${encodeURIComponent(accountId)}`)

  if (res.status === 404) {
    throw new Error(`Account "${accountId}" not found`)
  }
  if (!res.ok) {
    throw new Error(`Server returned ${res.status}`)
  }

  return res.json()
}
import type { TradeRequest, TradeResult } from "@/types/trade"

export async function submitTrade(body: TradeRequest): Promise<TradeResult> {
  const res = await fetch(`${API_BASE}/trades`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(body),
  })

  // Backend uses 400 for malformed input and 404 for missing IDs.
  // Both still return useful JSON; we treat them as application-level errors,
  // not thrown errors. Only true network failures throw.
  return res.json()
}
import type { TradesListResponse } from "@/types/trade"

export async function fetchRecentTrades(limit: number = 10): Promise<TradesListResponse> {
  const res = await fetch(`${API_BASE}/trades?limit=${limit}`)

  if (!res.ok) {
    throw new Error(`Server returned ${res.status}`)
  }

  return res.json()
}
import type { OhlcvBar } from "@/types/chart"

export async function fetchHistoricalBars(
  symbol: "nq" | "es",
  date: string,
): Promise<OhlcvBar[]> {
  const res = await fetch(`/data/${symbol}/${date}.json`)
  if (!res.ok) {
    throw new Error(`No data for ${symbol} on ${date}`)
  }
  return res.json()
}

export async function fetchAvailableDates(
  symbol: "nq" | "es",
): Promise<string[]> {
  const res = await fetch(`/data/${symbol}/index.json`)
  if (!res.ok) {
    throw new Error(`Index not found for ${symbol}`)
  }
  return res.json()
}