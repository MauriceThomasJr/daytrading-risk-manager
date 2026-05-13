import type { TradeFormState, Side, Instrument } from "@/types/trade"

interface TradeFormProps {
  state: TradeFormState
  onChange: (state: TradeFormState) => void
}

export function TradeForm({ state, onChange }: TradeFormProps) {
  function setInstrument(updates: Partial<Instrument>) {
    onChange({
      ...state,
      instrument: { ...state.instrument, ...updates },
    })
  }

  function setNumber(field: "entry_price" | "stop_price" | "target_price", value: string) {
    onChange({
      ...state,
      [field]: value === "" ? "" : parseFloat(value),
    })
  }

  return (
    <div className="rounded-lg border border-gray-200 bg-white p-5">
      <h2 className="mb-3 text-xs font-semibold uppercase tracking-wider text-gray-600">
        Trade
      </h2>

      <div className="space-y-2">
        <Field label="Symbol">
          <input
            type="text"
            value={state.instrument.symbol}
            onChange={(e) => setInstrument({ symbol: e.target.value })}
            className="flex-1 rounded border border-gray-300 px-2 py-1 text-sm"
          />
        </Field>

        <Field label="$/point">
          <input
            type="number"
            step="0.01"
            value={state.instrument.dollar_per_point}
            onChange={(e) =>
              setInstrument({ dollar_per_point: parseFloat(e.target.value) || 0 })
            }
            className="flex-1 rounded border border-gray-300 px-2 py-1 text-sm"
          />
        </Field>

        <Field label="Tick size">
          <input
            type="number"
            step="0.01"
            value={state.instrument.tick_size}
            onChange={(e) =>
              setInstrument({ tick_size: parseFloat(e.target.value) || 0 })
            }
            className="flex-1 rounded border border-gray-300 px-2 py-1 text-sm"
          />
        </Field>

        <Field label="Side">
          <select
            value={state.side}
            onChange={(e) => onChange({ ...state, side: e.target.value as Side })}
            className="flex-1 rounded border border-gray-300 px-2 py-1 text-sm"
          >
            <option value="Long">Long</option>
            <option value="Short">Short</option>
          </select>
        </Field>

        <Field label="Entry price">
          <input
            type="number"
            step="0.01"
            value={state.entry_price}
            onChange={(e) => setNumber("entry_price", e.target.value)}
            className="flex-1 rounded border border-gray-300 px-2 py-1 text-sm"
          />
        </Field>

        <Field label="Stop price">
          <input
            type="number"
            step="0.01"
            value={state.stop_price}
            onChange={(e) => setNumber("stop_price", e.target.value)}
            className="flex-1 rounded border border-gray-300 px-2 py-1 text-sm"
          />
        </Field>

        <Field label="Target (optional)">
          <input
            type="number"
            step="0.01"
            value={state.target_price}
            onChange={(e) => setNumber("target_price", e.target.value)}
            className="flex-1 rounded border border-gray-300 px-2 py-1 text-sm"
          />
        </Field>
      </div>
    </div>
  )
}

function Field({ label, children }: { label: string; children: React.ReactNode }) {
  return (
    <div className="flex gap-2 items-center">
      <label className="w-32 text-sm text-gray-600">{label}</label>
      {children}
    </div>
  )
}