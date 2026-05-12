#include "storage/InMemoryAccountStore.h"

void InMemoryAccountStore::save(const Account& account) {
    // operator[] inserts if missing, or overwrites if present.
    // But Account has no default constructor, so we can't use operator[].
    // Use insert_or_assign instead.
    accounts_.insert_or_assign(account.getAccountId(), account);
}

std::optional<Account> InMemoryAccountStore::load(const std::string& accountId) const {
    auto it = accounts_.find(accountId);
    if (it == accounts_.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool InMemoryAccountStore::exists(const std::string& accountId) const {
    return accounts_.find(accountId) != accounts_.end();
}

int InMemoryAccountStore::size() const {
    return static_cast<int>(accounts_.size());
}