#include "journal/SqliteTradeJournal.h"
#include "domain/Order.h"
#include <chrono>
#include <cstdint>

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

    // Seed the Order ID counter past whatever's already in the table,
    // so new orders don't collide with persisted ones.
    SQLite::Statement maxIdQuery(db_, "SELECT COALESCE(MAX(id), 0) FROM trades");
    if (maxIdQuery.executeStep()) {
        std::int64_t maxId = maxIdQuery.getColumn(0).getInt64();
        Order::seedNextOrderId(maxId);
    }
}

void SqliteTradeJournal::ensureSchema() {
    db_.exec(
        "CREATE TABLE IF NOT EXISTS trades ("
        "  id INTEGER PRIMARY KEY,"
        "  symbol TEXT NOT NULL,"
        "  dollar_per_point REAL NOT NULL,"
        "  tick_size REAL NOT NULL,"
        "  side TEXT NOT NULL,"
        "  size INTEGER NOT NULL,"
        "  entry_price REAL NOT NULL,"
        "  stop_price REAL NOT NULL,"
        "  target_price REAL,"
        "  created_at_ns INTEGER NOT NULL"
        ")"
    );

    // Add close-state columns if they don't already exist. ALTER TABLE
    // ADD COLUMN is the standard SQLite migration pattern — safe to run
    // every startup.
    addColumnIfMissing("closed_at_ns", "INTEGER");
    addColumnIfMissing("exit_price", "REAL");
    addColumnIfMissing("realized_pnl", "REAL");
}
void SqliteTradeJournal::addColumnIfMissing(const std::string& columnName,
                                            const std::string& columnType) {
    // Check if the column already exists by querying SQLite's table_info.
    SQLite::Statement query(db_,
        "SELECT COUNT(*) FROM pragma_table_info('trades') WHERE name = ?"
    );
    query.bind(1, columnName);
    query.executeStep();

    int existingCount = query.getColumn(0).getInt();
    if (existingCount == 0) {
        std::string sql = "ALTER TABLE trades ADD COLUMN " + columnName + " " + columnType;
        db_.exec(sql);
    }
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
    stmt.bind(10, static_cast<std::int64_t>(createdNs));

    stmt.exec();
}

std::vector<Order> SqliteTradeJournal::recentTrades(int limit) const {
    if (limit <= 0) {
        return {};
    }
    
    std::vector<Order> result;

    SQLite::Statement query(db_,
        "SELECT id, symbol, dollar_per_point, tick_size, side, size, "
        "       entry_price, stop_price, target_price, created_at_ns, "
        "       closed_at_ns, exit_price, realized_pnl "
        "FROM trades "
        "ORDER BY created_at_ns DESC "
        "LIMIT ?"
    );
    query.bind(1, limit);

    while (query.executeStep()) {
        std::int64_t id = query.getColumn(0).getInt64();
        std::string symbol = query.getColumn(1).getString();
        double dpp = query.getColumn(2).getDouble();
        double tick = query.getColumn(3).getDouble();
        std::string sideStr = query.getColumn(4).getString();
        int size = query.getColumn(5).getInt();
        double entryPrice = query.getColumn(6).getDouble();
        double stopPrice = query.getColumn(7).getDouble();

        std::optional<double> targetPrice;
        if (!query.isColumnNull(8)) {
            targetPrice = query.getColumn(8).getDouble();
        }

        std::int64_t createdAtNs = query.getColumn(9).getInt64();
        auto createdAt = std::chrono::system_clock::time_point(
            std::chrono::nanoseconds(createdAtNs));

        Instrument instrument(symbol, dpp, tick);
        Side side = (sideStr == "Long") ? Side::Long : Side::Short;
        TradeIntent intent(side, instrument, entryPrice, stopPrice, targetPrice);

        // Check whether this row is for a closed trade.
        if (!query.isColumnNull(10)) {
            std::int64_t closedAtNs = query.getColumn(10).getInt64();
            double exitPrice = query.getColumn(11).getDouble();
            double realizedPnL = query.getColumn(12).getDouble();
            auto closedAt = std::chrono::system_clock::time_point(
                std::chrono::nanoseconds(closedAtNs));

            result.push_back(Order::fromClosedStorage(
                intent, size, id, createdAt, closedAt, exitPrice, realizedPnL));
        } else {
            result.push_back(Order::fromStorage(intent, size, id, createdAt));
        }
    }

    return result;
}
void SqliteTradeJournal::closeTrade(std::int64_t orderId,
                                    double exitPrice,
                                    double realizedPnL,
                                    std::chrono::system_clock::time_point closedAt) {
    auto closedAtNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        closedAt.time_since_epoch()).count();

    SQLite::Statement stmt(db_,
        "UPDATE trades "
        "SET closed_at_ns = ?, exit_price = ?, realized_pnl = ? "
        "WHERE id = ?"
    );
    stmt.bind(1, static_cast<std::int64_t>(closedAtNs));
    stmt.bind(2, exitPrice);
    stmt.bind(3, realizedPnL);
    stmt.bind(4, orderId);

    int affected = stmt.exec();
    if (affected == 0) {
        throw std::runtime_error("No trade with ID " + std::to_string(orderId));
    }
}
std::optional<Order> SqliteTradeJournal::findById(std::int64_t orderId) const {
    SQLite::Statement query(db_,
        "SELECT id, symbol, dollar_per_point, tick_size, side, size, "
        "       entry_price, stop_price, target_price, created_at_ns, "
        "       closed_at_ns, exit_price, realized_pnl "
        "FROM trades "
        "WHERE id = ?"
    );
    query.bind(1, orderId);

    if (!query.executeStep()) {
        return std::nullopt;
    }

    std::int64_t id = query.getColumn(0).getInt64();
    std::string symbol = query.getColumn(1).getString();
    double dpp = query.getColumn(2).getDouble();
    double tick = query.getColumn(3).getDouble();
    std::string sideStr = query.getColumn(4).getString();
    int size = query.getColumn(5).getInt();
    double entryPrice = query.getColumn(6).getDouble();
    double stopPrice = query.getColumn(7).getDouble();

    std::optional<double> targetPrice;
    if (!query.isColumnNull(8)) {
        targetPrice = query.getColumn(8).getDouble();
    }

    std::int64_t createdAtNs = query.getColumn(9).getInt64();
    auto createdAt = std::chrono::system_clock::time_point(
        std::chrono::nanoseconds(createdAtNs));

    Instrument instrument(symbol, dpp, tick);
    Side side = (sideStr == "Long") ? Side::Long : Side::Short;
    TradeIntent intent(side, instrument, entryPrice, stopPrice, targetPrice);

    if (!query.isColumnNull(10)) {
        std::int64_t closedAtNs = query.getColumn(10).getInt64();
        double exitPrice = query.getColumn(11).getDouble();
        double realizedPnL = query.getColumn(12).getDouble();
        auto closedAt = std::chrono::system_clock::time_point(
            std::chrono::nanoseconds(closedAtNs));

        return Order::fromClosedStorage(
            intent, size, id, createdAt, closedAt, exitPrice, realizedPnL);
    }

    return Order::fromStorage(intent, size, id, createdAt);
}