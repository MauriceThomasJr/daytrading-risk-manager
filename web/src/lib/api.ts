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