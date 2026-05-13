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