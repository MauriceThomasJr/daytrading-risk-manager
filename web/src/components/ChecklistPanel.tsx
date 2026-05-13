import type { ChecklistResponses, ChecklistItem } from "@/types/trade"

interface ChecklistPanelProps {
  templateId: string
  onTemplateIdChange: (id: string) => void
  items: ChecklistItem[]
  responses: ChecklistResponses
  onResponsesChange: (responses: ChecklistResponses) => void
}

export function ChecklistPanel({
  templateId,
  onTemplateIdChange,
  items,
  responses,
  onResponsesChange,
}: ChecklistPanelProps) {
  function toggleItem(itemId: string, checked: boolean) {
    onResponsesChange({ ...responses, [itemId]: checked })
  }

  return (
    <div className="rounded-lg border border-gray-200 bg-white p-5">
      <h2 className="mb-3 text-xs font-semibold uppercase tracking-wider text-gray-600">
        Checklist
      </h2>

      <div className="flex gap-2 items-center mb-4">
        <label htmlFor="template-id" className="w-32 text-sm text-gray-600">
          Template ID
        </label>
        <input
          id="template-id"
          type="text"
          value={templateId}
          onChange={(e) => onTemplateIdChange(e.target.value)}
          className="flex-1 rounded border border-gray-300 px-2 py-1 text-sm"
        />
      </div>

      <div className="space-y-2">
        {items.map((item) => (
          <label
            key={item.id}
            className="flex items-center gap-2 text-sm cursor-pointer"
          >
            <input
              type="checkbox"
              checked={responses[item.id] ?? false}
              onChange={(e) => toggleItem(item.id, e.target.checked)}
              className="w-4 h-4 rounded border-gray-300"
            />
            <span className="text-gray-700">{item.prompt}</span>
          </label>
        ))}
      </div>
    </div>
  )
}