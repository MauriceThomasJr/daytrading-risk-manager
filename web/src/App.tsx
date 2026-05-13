import { AccountPanel } from "@/components/AccountPanel"

function App() {
  return (
    <div className="min-h-screen bg-gray-50 p-8">
      <div className="mx-auto max-w-3xl">
        <h1 className="text-2xl font-bold text-gray-900">
          Daytrading Risk Manager
        </h1>
        <p className="mt-1 text-sm text-gray-600">
          Pre-trade discipline gate over the local API.
        </p>

        <div className="mt-6">
          <AccountPanel />
        </div>
      </div>
    </div>
  )
}

export default App