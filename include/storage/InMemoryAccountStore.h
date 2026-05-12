#ifndef IN_MEMORY_ACCOUNT_STORE_H
#define IN_MEMORY_ACCOUNT_STORE_H

#include "storage/IAccountStore.h"
#include <unordered_map>

class InMemoryAccountStore : public IAccountStore {
public:
    void save(const Account& account) override;
    std::optional<Account> load(const std::string& accountId) const override;
    bool exists(const std::string& accountId) const override;

    int size() const;

private:
    std::unordered_map<std::string, Account> accounts_;
};

#endif