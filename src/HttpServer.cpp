#include "http/HttpServer.h"
#include "domain/Account.h"

#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

namespace {
    // Convert an Account to a JSON object.
    json accountToJson(const Account& account) {
        return json{
            {"account_id",   account.getAccountId()},
            {"balance",      account.getBalance()},
            {"daily_pnl",    account.getDailyPnL()},
            {"trades_today", account.getTradesToday()}
        };
    }
}

HttpServer::HttpServer(IAccountStore& accountStore)
    : accountStore_(accountStore) {
    registerRoutes();
}

void HttpServer::registerRoutes() {
    // Health check.
    server_.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {{"status", "ok"}};
        res.set_content(response.dump(), "application/json");
    });

    // GET /accounts/{id} -- load an account by ID.
    server_.Get(R"(/accounts/([^/]+))",
                [this](const httplib::Request& req, httplib::Response& res) {
        std::string accountId = req.matches[1];

        auto account = accountStore_.load(accountId);
        if (!account) {
            res.status = 404;
            json error = {
                {"error", "Account not found"},
                {"account_id", accountId}
            };
            res.set_content(error.dump(), "application/json");
            return;
        }

        res.set_content(accountToJson(*account).dump(), "application/json");
    });
}

void HttpServer::listen(const std::string& host, int port) {
    std::cout << "Server listening on http://" << host << ":" << port << "\n";
    server_.listen(host, port);
}