import { useMutation, useQueryClient } from "@tanstack/react-query"
import { submitTrade } from "@/lib/api"
import { Button } from "@/components/ui/button"
import type {
  TradeFormState,
  ChecklistResponses,
  TradeRequest,
  TradeResult,
} from "@/types/trade"

interface SubmitBarProps {
  accountId: string
  templateId: string
  trade: TradeFormState
  responses: ChecklistResponses
}

export function SubmitBar({
  accountId,
  templateId,
  trade,
  responses,
}: SubmitBarProps) {
  const queryClient = useQueryClient()

  const mutation = useMutation<TradeResult, Error, TradeRequest>({
    mutationFn: submitTrade,
    onSuccess: (data) => {
      // Refresh the account if the trade was accepted (the count went up).
      if (data.accepted) {
        queryClient.invalidateQueries({
          queryKey: ["account", accountId],
        })
      }
    },
  })

  function handleSubmit() {
    // Validate that prices are filled in.
    if (trade.entry_price === "" || trade.stop_price === "") {
      alert("Entry and stop prices are required.")
      return
    }

    const body: TradeRequest = {
      account_id: accountId,
      template_id: templateId,
      side: trade.side,
      entry_price: trade.entry_price,
      stop_price: trade.stop_price,
      instrument: trade.instrument,
      checklist_responses: responses,
    }

    if (trade.target_price !== "") {
      body.target_price = trade.target_price
    }

    mutation.mutate(body)
  }

  return (
    <div className="rounded-lg border border-gray-200 bg-white p-5">
      <h2 className="mb-3 text-xs font-semibold uppercase tracking-wider text-gray-600">
        Submit
      </h2>

      <Button onClick={handleSubmit} disabled={mutation.isPending}>
        {mutation.isPending ? "Submitting..." : "Submit Trade"}
      </Button>

      <div className="mt-4 min-h-[60px]">
        {mutation.isError && (
          <ResultMessage variant="rejected">
            Network error: {mutation.error.message}
          </ResultMessage>
        )}

        {mutation.isSuccess && <ResultDisplay result={mutation.data} />}
      </div>
    </div>
  )
}

function ResultDisplay({ result }: { result: TradeResult }) {
  // Backend uses 4xx with a JSON error body for "account not found" etc.
  if (result.error) {
    return (
      <ResultMessage variant="rejected">
        {result.error}
        {result.account_id && ` (account_id: ${result.account_id})`}
        {result.template_id && ` (template_id: ${result.template_id})`}
      </ResultMessage>
    )
  }

  if (result.accepted && result.order) {
    const o = result.order
    return (
      <ResultMessage variant="accepted">
        ✓ ACCEPTED — Order #{o.id}: {o.size} {o.side} {o.symbol} @ {o.entry_price}, stop {o.stop_price}
      </ResultMessage>
    )
  }

  const reasons = result.rejection_reasons?.join("; ") ?? "Unknown reason"
  return (
    <ResultMessage variant="rejected">
      ✗ REJECTED — {reasons}
    </ResultMessage>
  )
}

function ResultMessage({
  variant,
  children,
}: {
  variant: "accepted" | "rejected"
  children: React.ReactNode
}) {
  const styles =
    variant === "accepted"
      ? "bg-green-50 border-green-200 text-green-800"
      : "bg-red-50 border-red-200 text-red-800"

  return (
    <div className={`rounded border p-3 text-sm font-medium ${styles}`}>
      {children}
    </div>
  )
}