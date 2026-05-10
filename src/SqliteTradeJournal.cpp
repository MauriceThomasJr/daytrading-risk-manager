#include "journal/SqliteTradeJournal.h"

#include <chrono>

namespace {
    void bindOptional(SQLite::Statement& stmt, int index, std::optional<double> value) {
        if (value.has_value()) {
            stmt.bind(index, *value);
        } else {
            stmt.bind(index);   // No second arg = bind NULL
        }
    }

    const char* sideToString(Side s) {
        return (s == Side::Long) ? "Long" : "Short";
    }

    Side sideFromString(const std::string& s) {
        return (s == "Long") ? Side::Long : Side::Short;
    }
}

SqliteTradeJournal::SqliteTradeJournal(const std::string& dbPath)
    : db_(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    ensureSchema();
}

void SqliteTradeJournal::ensureSchema() {
    db_.exec(
        "CREATE TABLE IF NOT EXISTS trades ("
        "  id              INTEGER PRIMARY KEY,"
        "  symbol          TEXT    NOT NULL,"
        "  dollar_per_point REAL   NOT NULL,"
        "  tick_size       REAL    NOT NULL,"
        "  side            TEXT    NOT NULL,"
        "  entry_price     REAL    NOT NULL,"
        "  stop_price      REAL    NOT NULL,"
        "  target_price    REAL,"
        "  size            INTEGER NOT NULL,"
        "  created_at_ns   INTEGER NOT NULL"
        ")"
    );
}

void SqliteTradeJournal::record(const Order& order) {
    SQLite::Statement stmt(db_,
        "INSERT INTO trades "
        "(id, symbol, dollar_per_point, tick_size, side, "
        " entry_price, stop_price, target_price, size, created_at_ns) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );

    const auto& instrument = order.getInstrument();
    auto createdNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        order.getCreatedAt().time_since_epoch()).count();

    stmt.bind(1, order.getId());
    stmt.bind(2, instrument.getSymbol());
    stmt.bind(3, instrument.getDollarPerPoint());
    stmt.bind(4, instrument.getTickSize());
    stmt.bind(5, sideToString(order.getSide()));
    stmt.bind(6, order.getEntryPrice());
    stmt.bind(7, order.getStopPrice());
    bindOptional(stmt, 8, order.getTargetPrice());
    stmt.bind(9, order.getSize());
    stmt.bind(10, static_cast<long long>(createdNs));

    stmt.exec();
}

std::vector<Order> SqliteTradeJournal::recentTrades(int limit) const {
    std::vector<Order> result;
    if (limit <= 0) return result;

    SQLite::Statement query(db_,
        "SELECT id, symbol, dollar_per_point, tick_size, side, "
        "       entry_price, stop_price, target_price, size, created_at_ns "
        "FROM trades "
        "ORDER BY created_at_ns DESC "
        "LIMIT ?"
    );
    query.bind(1, limit);

    while (query.executeStep()) {
        Instrument instrument(
            query.getColumn(1).getString(),
            query.getColumn(2).getDouble(),
            query.getColumn(3).getDouble()
        );

        std::optional<double> target;
        if (!query.getColumn(7).isNull()) {
            target = query.getColumn(7).getDouble();
        }

        TradeIntent intent(
            sideFromString(query.getColumn(4).getString()),
            instrument,
            query.getColumn(5).getDouble(),
            query.getColumn(6).getDouble(),
            target
        );

        std::int64_t id = query.getColumn(0).getInt64();
        int size = query.getColumn(8).getInt();
        auto ns = std::chrono::nanoseconds(query.getColumn(9).getInt64());
        auto createdAt = std::chrono::system_clock::time_point(ns);

        result.push_back(Order::fromStorage(intent, size, id, createdAt));
    }

    return result;
}