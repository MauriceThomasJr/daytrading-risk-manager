# Daytrading Risk Manager

[![CI](https://github.com/MauriceThomasJr/daytrading-risk-manager/actions/workflows/ci.yml/badge.svg)](https://github.com/MauriceThomasJr/daytrading-risk-manager/actions/workflows/ci.yml)

A C++17 trading discipline engine that gates orders behind a user-defined
checklist and configurable risk rules, with persistent trade journaling
via SQLite. Built as a passion project and portfolio piece exploring
modern C++ design, clean architecture, and test-driven development.

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

The codebase is organized into five bounded contexts:

- **`domain/`** — value types describing the trading world: `Account`,
  `Instrument`, `Side`, `TradeIntent`, `Order`
- **`risk/`** — `RiskManager`, the rule layer: position sizing based on
  per-trade risk percentage, daily loss limits, max trade count
- **`checklist/`** — user-defined pre-trade discipline: `ChecklistItem`,
  `ChecklistTemplate`, `ChecklistGate`
- **`journal/`** — persistent trade journaling: `ITradeJournal` interface
  with `InMemoryTradeJournal` and `SqliteTradeJournal` implementations
- **`pipeline/`** — `TradePipeline`, the orchestrator that runs every
  trade through the checklist and risk gates, sizes it, builds an
  `Order`, and records it to the journal

Trade flow:

```
TradeIntent → ChecklistGate → RiskManager → Order → ITradeJournal
                  ↓               ↓
              rejected        rejected
```

The pipeline is the only path that produces a sized `Order`, so
discipline is enforced by the architecture rather than by developer
convention. Every accepted trade is automatically journaled.

## Building

Requires CMake 3.14+ and a C++17 compiler.

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

External dependencies (Catch2 and SQLiteCpp) are fetched and built
automatically — no separate install required.

## Running

The example program demonstrates the full pipeline end-to-end with a
persistent SQLite journal:

```bash
./basic_usage
```

This will submit several trades (some accepted, some rejected by the
checklist), record accepted ones to a `trades.db` SQLite file in the
working directory, and print a summary. Run it again to see persistence
in action — previously journaled trades are still there.

Run the test suite:

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
│   ├── journal/
│   └── pipeline/
├── src/                    Implementation files
├── tests/                  Catch2 unit and integration tests
├── examples/               Example usage programs
└── CMakeLists.txt          Build configuration
```

## Design Highlights

- **Stateless rule classes.** `RiskManager` and `ChecklistGate` carry no
  per-account state. They take an `Account` and a checklist response as
  parameters and return a verdict. One rule set can apply to many
  accounts; tests are trivial to write.

- **Type-safe lifecycle.** `TradeIntent` (what the user wants) and
  `Order` (what gets sent) are deliberately separate types. An `Order`
  can only be constructed through static factory methods, enforcing
  "you must validate before sending" at the type-system level.

- **Polymorphic persistence.** `TradePipeline` depends on an abstract
  `ITradeJournal` interface, not on SQLite directly. Two concrete
  implementations are provided — `InMemoryTradeJournal` for tests,
  `SqliteTradeJournal` for production — and either can be swapped in
  via dependency injection.

- **Composition over duplication.** `Order` holds an internal
  `TradeIntent` and delegates shared field access to it, avoiding
  duplicate getters between the two types.

- **Modern C++.** Uses `std::optional` for nullable fields, `std::chrono`
  for timestamps, `std::unordered_map` and `std::vector` for collections,
  `enum class` for type-safe enumerations, atomic counters for thread-
  safe ID generation, and member initializer lists throughout.

## Test Coverage

The project ships with a Catch2 test suite covering each class in
isolation plus full pipeline integration. Tests include round-trip
verification through the SQLite journal (write, close, reopen, read)
and contract verification through the abstract `ITradeJournal`
interface. All tests run via `ctest --output-on-failure` after building.

Continuous integration runs the full build and test suite on Ubuntu via
GitHub Actions on every push.

## Roadmap

**Implemented**
- Core domain types (`Account`, `Instrument`, `Side`, `TradeIntent`, `Order`)
- Risk management with position sizing and daily loss limits
- User-defined checklist with gating logic
- Trade pipeline orchestrating checklist + risk + sizing
- Persistent trade journal via SQLite (`ITradeJournal` interface)
- Catch2 test suite
- CMake build system with FetchContent dependencies
- GitHub Actions CI

**Planned**
- `BrokerAdapter` abstraction with mock implementation for paper trading
- Persistence for accounts and checklist templates
- HTTP API layer
- Web frontend with TradingView chart integration
- Real broker integration (Tradovate / Interactive Brokers)
- Trade journal analytics — which checklist criteria correlate with
  successful trades
- Backtester that replays historical data against the trader's checklist
- Single-session enforcement to prevent rule circumvention

## License

MIT