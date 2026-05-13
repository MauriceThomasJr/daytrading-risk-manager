# Daytrading Risk Manager

[![CI](https://github.com/MauriceThomasJr/daytrading-risk-manager/actions/workflows/ci.yml/badge.svg)](https://github.com/MauriceThomasJr/daytrading-risk-manager/actions/workflows/ci.yml)

A C++17 trading discipline engine exposed over REST. Trades flow through
a user-defined pre-trade checklist, configurable risk rules, a broker
adapter, and a persistent SQLite journal — with no path to execution
that skips any gate. Built as a passion project and portfolio piece
exploring modern C++ design, clean architecture, and test-driven
development.

## The Problem

Most prop trading platforms attempt risk management by treating the
symptom: post-loss lockouts, max-drawdown alerts, daily loss caps. These
help, but they don't address the upstream issue — that traders enter
trades without verifying their own thesis in the first place.

This project flips the model. Before any order can be placed, the trader
must satisfy their own pre-trade checklist: entry parameters, points of
interest, confirmations, risk parameters. The checklist is fully
customizable to the trader's strategy, and the rules are enforced
structurally — there is no path through the code that produces an order
without first passing every gate.

The side effect: every completed checklist is a record of what a "good
trade" looks like for that trader. That data persists in a SQLite trade
journal, ready for review, analysis, and (eventually) backtesting
against the trader's own criteria.

## Architecture

The codebase is organized into seven bounded contexts:

- **`domain/`** — value types describing the trading world: `Account`,
  `Instrument`, `Side`, `TradeIntent`, `Order`
- **`risk/`** — `RiskManager`, the rule layer: position sizing based on
  per-trade risk percentage, daily loss limits, max trade count
- **`checklist/`** — user-defined pre-trade discipline: `ChecklistItem`,
  `ChecklistTemplate`, `ChecklistGate`
- **`broker/`** — `IBrokerAdapter` interface with `MockBrokerAdapter`
  implementation; ready to plug in real broker integrations
- **`journal/`** — persistent trade journaling: `ITradeJournal` interface
  with `InMemoryTradeJournal` and `SqliteTradeJournal` implementations
- **`storage/`** — account and checklist template persistence:
  `IAccountStore`, `IChecklistTemplateStore`, each with in-memory and
  SQLite implementations
- **`pipeline/`** — `TradePipeline`, the orchestrator that runs every
  trade through every gate before producing an `Order`
- **`http/`** — `HttpServer`, the REST API layer over the pipeline

Trade flow:

```
TradeIntent → ChecklistGate → RiskManager → Broker → Journal
                  ↓               ↓           ↓
              rejected        rejected   rejected
```

The pipeline is the only path that produces a sized `Order`, so
discipline is enforced by the architecture rather than by developer
convention. Every accepted trade is automatically routed to the broker
and journaled.

## API

The HTTP server exposes the pipeline over REST. Default port: `8080`.

### `GET /health`
Service liveness check.

```bash
curl http://localhost:8080/health
# {"status":"ok"}
```

### `GET /accounts/{id}`
Load an account's current state.

```bash
curl http://localhost:8080/accounts/alice
# {"account_id":"alice","balance":49900.0,"daily_pnl":-100.0,"trades_today":1}
```

Returns `404` if no account with that ID exists.

### `POST /trades`
Submit a trade for evaluation through the full pipeline.

Request body:
```json
{
    "account_id": "alice",
    "template_id": "daily-es",
    "side": "Long",
    "entry_price": 7000.0,
    "stop_price": 6991.0,
    "target_price": 7050.0,
    "instrument": {
        "symbol": "MES",
        "dollar_per_point": 5.0,
        "tick_size": 0.25
    },
    "checklist_responses": {
        "volume": true,
        "htf": true,
        "plan": true
    }
}
```

Accepted response:
```json
{
    "accepted": true,
    "order": {
        "id": 5,
        "side": "Long",
        "size": 22,
        "symbol": "MES",
        "entry_price": 7000.0,
        "stop_price": 6991.0,
        "target_price": 7050.0
    }
}
```

Rejected response:
```json
{
    "accepted": false,
    "rejection_reasons": ["Is this trade in your written plan?"]
}
```

Returns `400` for malformed JSON or missing fields, `404` for unknown
account or template IDs.

### `GET /trades?limit=N`
List recent trades from the journal, newest first. `limit` defaults to
10 and is optional.

```bash
curl http://localhost:8080/trades?limit=3
# {"trades":[{"id":12,...},{"id":11,...},{"id":10,...}]}
```

## Building

Requires CMake 3.14+ and a C++17 compiler.

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

External dependencies (Catch2, SQLiteCpp, cpp-httplib, nlohmann/json)
are fetched and built automatically — no separate install required.

## Running

Three executables are built:

**`http_server`** — starts the REST API on port 8080.

```bash
./http_server
```

Creates `accounts.db` (accounts + checklist templates) and `trades.db`
(trade journal) in the working directory. Seeds demo accounts
(`alice`, `bob`) and a `daily-es` checklist template on first run.

**`basic_usage`** — runs the trade pipeline end-to-end as a CLI demo,
recording trades to a local SQLite file.

```bash
./basic_usage
```

**`tests`** — the test suite. Run via CTest for cleaner output:

```bash
ctest --output-on-failure
```

## Project Structure

```
daytrading-risk-manager/
├── .github/workflows/      CI configuration
├── include/                Public headers, organized by context
│   ├── domain/
│   ├── risk/
│   ├── checklist/
│   ├── broker/
│   ├── journal/
│   ├── storage/
│   ├── pipeline/
│   └── http/
├── src/                    Implementation files
├── tests/                  Catch2 unit and integration tests
├── examples/               Example usage programs (basic_usage.cpp)
└── CMakeLists.txt          Build configuration
```

## Design Highlights

- **Stateless rule classes.** `RiskManager` and `ChecklistGate` carry no
  per-account state. They take an `Account` and a checklist response as
  parameters and return a verdict.

- **Type-safe lifecycle.** `TradeIntent` (what the user wants) and
  `Order` (what gets sent) are deliberately separate types. An `Order`
  can only be constructed through static factory methods, enforcing
  "you must validate before sending" at the type-system level.

- **Polymorphic external boundaries.** Three abstract interfaces
  (`ITradeJournal`, `IBrokerAdapter`, `IAccountStore` /
  `IChecklistTemplateStore`) each have multiple implementations — one
  in-memory for tests, one SQLite-backed for production. The pipeline
  and HTTP server depend on the interfaces, not the implementations.

- **Composition over duplication.** `Order` holds an internal
  `TradeIntent` and delegates shared field access to it.

- **Transactions for multi-table writes.** Saving a checklist template
  updates two tables; the SQLite implementation wraps the work in a
  transaction so partial writes can't corrupt state.

- **Modern C++.** Uses `std::optional` for nullable fields,
  `std::chrono` for timestamps, `std::unordered_map` and `std::vector`
  for collections, `enum class` for type-safe enumerations, atomic
  counters for thread-safe ID generation, and member initializer lists
  throughout.

## Test Coverage

The project ships with a Catch2 test suite covering each class in
isolation plus full pipeline integration. Tests include round-trip
verification through all SQLite-backed stores (write, close, reopen,
read) and contract verification through every abstract interface. All
tests run via `ctest --output-on-failure` after building.

Continuous integration runs the full build and test suite on Ubuntu via
GitHub Actions on every push.

## Roadmap

**Implemented**
- Core domain types (`Account`, `Instrument`, `Side`, `TradeIntent`,
  `Order`)
- Risk management with position sizing and daily loss limits
- User-defined checklist with gating logic
- Trade pipeline orchestrating checklist + risk + broker + journal
- `IBrokerAdapter` interface with mock implementation
- Persistent trade journal via SQLite
- Persistent account and checklist template stores via SQLite
- REST API exposing the pipeline over HTTP
- Catch2 test suite with ~340 assertions across ~96 test cases
- CMake build system with FetchContent dependencies
- GitHub Actions CI

**Planned**
- Web frontend with TradingView chart integration
- Real broker integration (Tradovate / Interactive Brokers paper)
- API endpoints for creating accounts and templates
- Authentication and per-user account isolation
- Trade journal analytics — which checklist criteria correlate with
  successful trades
- Backtester that replays historical data against the trader's
  checklist
- Cross-platform CI (Windows + macOS in addition to Linux)

## License

MIT