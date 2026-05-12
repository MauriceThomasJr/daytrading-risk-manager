#ifndef SQLITE_ACCOUNT_STORE_H
#define SQLITE_ACCOUNT_STORE_H

#include "storage/IAccountStore.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

class SqliteAccountStore : public IAccountStore {
public:
    explicit SqliteAccountStore(const std::string& dbPath);

    void save(const Account& account) override;
    std::optional<Account> load(const std::string& accountId) const override;
    bool exists(const std::string& accountId) const override;

private:
    void ensureSchema();

    mutable SQLite::Database db_;
};

#endif