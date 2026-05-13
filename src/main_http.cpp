// src/main_http.cpp
//
// HTTP server entry point.
// Loads accounts from a SQLite database and exposes them over REST.

#include "http/HttpServer.h"
#include "storage/SqliteAccountStore.h"
#include "domain/Account.h"

#include <iostream>

int main() {
    // Use a file-backed database so accounts persist across runs.
    SqliteAccountStore accountStore("accounts.db");

    // Seed with a couple of demo accounts if they don't already exist.
    // In a real product, accounts would be created via a registration endpoint.
    if (!accountStore.exists("alice")) {
        Account alice("alice", 50000.0);
        alice.recordTradeResult(-100.0);
        accountStore.save(alice);
    }
    if (!accountStore.exists("bob")) {
        accountStore.save(Account("bob", 100000.0));
    }

    HttpServer server(accountStore);
    server.listen("0.0.0.0", 8080);

    return 0;
}