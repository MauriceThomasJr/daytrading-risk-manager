#ifndef IACCOUNT_STORE_H
#define IACCOUNT_STORE_H

#include "domain/Account.h"
#include <optional>
#include <string>

class IAccountStore {
public:
    virtual ~IAccountStore() = default;

    // Store an account, or update it if its ID already exists.
    virtual void save(const Account& account) = 0;

    // Load an account by ID. Returns nullopt if no account with that ID exists.
    virtual std::optional<Account> load(const std::string& accountId) const = 0;

    // Check whether an account with this ID exists, without loading it.
    virtual bool exists(const std::string& accountId) const = 0;
};

#endif