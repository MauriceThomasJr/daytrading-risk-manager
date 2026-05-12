#include "storage/SqliteAccountStore.h"

SqliteAccountStore::SqliteAccountStore(const std::string& dbPath)
    : db_(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    ensureSchema();
}

void SqliteAccountStore::ensureSchema() {
    db_.exec(
        "CREATE TABLE IF NOT EXISTS accounts ("
        "  account_id    TEXT    PRIMARY KEY,"
        "  balance       REAL    NOT NULL,"
        "  daily_pnl     REAL    NOT NULL,"
        "  trades_today  INTEGER NOT NULL"
        ")"
    );
}

void SqliteAccountStore::save(const Account& account) {
    SQLite::Statement stmt(db_,
        "INSERT OR REPLACE INTO accounts "
        "(account_id, balance, daily_pnl, trades_today) "
        "VALUES (?, ?, ?, ?)"
    );

    stmt.bind(1, account.getAccountId());
    stmt.bind(2, account.getBalance());
    stmt.bind(3, account.getDailyPnL());
    stmt.bind(4, account.getTradesToday());

    stmt.exec();
}

std::optional<Account> SqliteAccountStore::load(const std::string& accountId) const {
    SQLite::Statement query(db_,
        "SELECT account_id, balance, daily_pnl, trades_today "
        "FROM accounts WHERE account_id = ?"
    );
    query.bind(1, accountId);

    if (!query.executeStep()) {
        return std::nullopt;
    }

    return Account::fromStorage(
        query.getColumn(0).getString(),
        query.getColumn(1).getDouble(),
        query.getColumn(2).getDouble(),
        query.getColumn(3).getInt()
    );
}

bool SqliteAccountStore::exists(const std::string& accountId) const {
    SQLite::Statement query(db_,
        "SELECT 1 FROM accounts WHERE account_id = ? LIMIT 1"
    );
    query.bind(1, accountId);

    return query.executeStep();
}