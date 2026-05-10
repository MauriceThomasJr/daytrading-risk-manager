# Daytrading Risk Manager

[![CI](https://github.com/MauriceThomasJr/daytrading-risk-manager/actions/workflows/ci.yml/badge.svg)](https://github.com/MauriceThomasJr/daytrading-risk-manager/actions/workflows/ci.yml)

A C++17 trading discipline engine that gates orders behind a user-defined
checklist and configurable risk rules. Built as a passion project and
portfolio piece exploring modern C++ design, clean architecture, and
test-driven development.

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
trade" looks like for that trader. That data carries forward into
journaling, analysis, and (eventually) backtesting against the trader's
own criteria.

## Architecture

The codebase is organized into four bounded contexts:

- **`domain/`** — value types describing the trading world: `Account`,
  `Instrument`, `Side`, `TradeIntent`, `Order`
- **`risk/`** — `RiskManager`, the rule layer: position sizing based on
  per-trade risk percentage, daily loss limits, max trade count
- **`checklist/`** — user-defined pre-trade discipline: `ChecklistItem`,
  `ChecklistTemplate`, `ChecklistGate`
- **`pipeline/`** — `TradePipeline`, the orchestrator that runs every
  trade through the checklist and risk gates before producing an `Order`

Trade flow:

```
TradeIntent → ChecklistGate → RiskManager.canTakeNewTrade
                  ↓               ↓
              rejected        rejected
                                  ↓
                           RiskManager.calculatePositionSize
                                  ↓
                              Order (only here)
```

The pipeline is the only path that produces a sized `Order`, so
discipline is enforced by the architecture rather than by developer
convention.

## Building

Requires CMake 3.14+ and a C++17 compiler.

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Run the example program:

```bash
./basic_usage
```

Run the test suite:

```bash
./tests
# or with cleaner output:
ctest --output-on-failure
```

Catch2 is fetched and built automatically by CMake — no separate install
required.

## Project Structure

```
daytrading-risk-manager/
├── include/                 Public headers, organized by context
│   ├── domain/
│   ├── risk/
│   ├── checklist/
│   └── pipeline/
├── src/                     Implementation files
├── tests/                   Catch2 unit and integration tests
├── examples/                Example usage programs
└── CMakeLists.txt           Build configuration
```

## Design Highlights

- **Stateless rule classes.** `RiskManager` and `ChecklistGate` carry no
  per-account state. They take an `Account` and a checklist response as
  parameters and return a verdict. One rule set can apply to many
  accounts; tests are trivial to write.

- **Type-safe lifecycle.** `TradeIntent` (what the user wants) and
  `Order` (what gets sent) are deliberately separate types. An `Order`
  can only be constructed through a static factory that takes a
  validated intent, so the type system enforces "you must validate
  before sending."

- **Composition over duplication.** `Order` holds an internal
  `TradeIntent` and delegates shared field access to it, avoiding
  duplicate getters between the two types.

- **Modern C++.** Uses `std::optional`, `std::unordered_map`,
  `std::vector`, `enum class` for type-safe enumerations, and member
  initializer lists throughout.

## Test Coverage

The project ships with a Catch2 test suite covering each class in
isolation as well as full pipeline integration. Tests are run with
`ctest --output-on-failure` after building.

## Roadmap

**Implemented**
- Core domain types (`Account`, `Instrument`, `Side`, `TradeIntent`, `Order`)
- Risk management with position sizing and daily loss limits
- User-defined checklist with gating logic
- Trade pipeline orchestrating checklist + risk gates
- Catch2 test suite
- CMake build system with FetchContent

**Planned**
- `BrokerAdapter` abstraction with mock implementation for paper trading
- Persistence layer (SQLite) for accounts, templates, and trade history
- HTTP API layer
- Web frontend with TradingView chart integration
- Broker integration (Tradovate / Interactive Brokers)
- Trade journal with analysis of which checklist criteria correlate with
  successful trades
- Backtester that replays historical data against the trader's checklist
- Single-session enforcement to prevent rule circumvention

## License

MIT