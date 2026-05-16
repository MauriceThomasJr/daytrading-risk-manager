#include "http/HttpServer.h"
#include "domain/Account.h"
#include "domain/Instrument.h"
#include "domain/Side.h"
#include "domain/TradeIntent.h"
#include "domain/Order.h"
#include "checklist/ChecklistTemplate.h"
#include "checklist/ChecklistResponse.h"
#include "pipeline/TradeSubmissionResult.h"

#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

namespace {

// ---------------------------------------------------------------------
// JSON serialization (C++ -> JSON)
// ---------------------------------------------------------------------

const char* sideToString(Side s) {
    return (s == Side::Long) ? "Long" : "Short";
}

json accountToJson(const Account& account) {
    return json{
        {"account_id",   account.getAccountId()},
        {"balance",      account.getBalance()},
        {"daily_pnl",    account.getDailyPnL()},
        {"trades_today", account.getTradesToday()}
    };
}

json orderToJson(const Order& order) {
    json j = {
        {"id",           order.getId()},
        {"size",         order.getSize()},
        {"side",         sideToString(order.getSide())},
        {"symbol",       order.getInstrument().getSymbol()},
        {"entry_price",  order.getEntryPrice()},
        {"stop_price",   order.getStopPrice()}
    };
    if (order.getTargetPrice().has_value()) {
        j["target_price"] = *order.getTargetPrice();
    }
    if (order.isClosed()) {
        j["is_closed"] = true;
        j["exit_price"] = *order.getExitPrice();
        j["realized_pnl"] = *order.getRealizedPnL();
    } else {
        j["is_closed"] = false;
    }
    return j;
}

json submissionResultToJson(const TradeSubmissionResult& result) {
    json j = {{"accepted", result.accepted}};
    if (result.accepted && result.order.has_value()) {
        j["order"] = orderToJson(*result.order);
    } else {
        j["rejection_reasons"] = result.rejectionReasons;
    }
    return j;
}

// ---------------------------------------------------------------------
// JSON deserialization (JSON -> C++)
// ---------------------------------------------------------------------

Side sideFromString(const std::string& s) {
    if (s == "Long")  return Side::Long;
    if (s == "Short") return Side::Short;
    throw std::invalid_argument("Invalid side: " + s);
}

// Helper to build error JSON responses.
void writeError(httplib::Response& res, int status,
                const std::string& message,
                const std::string& detailKey = "",
                const std::string& detailValue = "") {
    json error = {{"error", message}};
    if (!detailKey.empty()) {
        error[detailKey] = detailValue;
    }
    res.status = status;
    res.set_content(error.dump(), "application/json");
}

}  // anonymous namespace

// ---------------------------------------------------------------------
// HttpServer implementation
// ---------------------------------------------------------------------

HttpServer::HttpServer(IAccountStore& accountStore,
                       IChecklistTemplateStore& templateStore,
                       TradePipeline& pipeline,
                       ITradeJournal& journal)
    : accountStore_(accountStore),
      templateStore_(templateStore),
      pipeline_(pipeline),
      journal_(journal) {
    registerRoutes();
}

void HttpServer::registerRoutes() {

    // Allow browser-based clients (the frontend) to call this API.
    server_.set_pre_routing_handler([](const httplib::Request&,
                                        httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // Respond to CORS preflight requests (OPTIONS) for any path.
    server_.Options(R"(.*)",
                    [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    // ----- Health -----
    server_.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {{"status", "ok"}};
        res.set_content(response.dump(), "application/json");
    });

    // ----- GET /accounts/{id} -----
    server_.Get(R"(/accounts/([^/]+))",
                [this](const httplib::Request& req, httplib::Response& res) {
        std::string accountId = req.matches[1];

        auto account = accountStore_.load(accountId);
        if (!account) {
            writeError(res, 404, "Account not found", "account_id", accountId);
            return;
        }

        res.set_content(accountToJson(*account).dump(), "application/json");
    });

    // ----- POST /trades -----
    server_.Post("/trades",
                 [this](const httplib::Request& req, httplib::Response& res) {
        // 1. Parse the request body.
        json body;
        try {
            body = json::parse(req.body);
        } catch (const json::parse_error&) {
            writeError(res, 400, "Invalid JSON in request body");
            return;
        }

        try {
            std::string accountId  = body.at("account_id").get<std::string>();
            std::string templateId = body.at("template_id").get<std::string>();
            std::string sideStr    = body.at("side").get<std::string>();
            double currentPrice    = body.at("current_price").get<double>();
            double stopPrice       = body.at("stop_price").get<double>();

            auto accountOpt = accountStore_.load(accountId);
            if (!accountOpt) {
                writeError(res, 404, "Account not found", "account_id", accountId);
                return;
            }
            Account account = *accountOpt;

            auto templateOpt = templateStore_.load(templateId);
            if (!templateOpt) {
                writeError(res, 404, "Checklist template not found",
                           "template_id", templateId);
                return;
            }
            ChecklistTemplate tmpl = *templateOpt;

            json instrumentJson = body.at("instrument");
            Instrument instrument(
                instrumentJson.at("symbol").get<std::string>(),
                instrumentJson.at("dollar_per_point").get<double>(),
                instrumentJson.at("tick_size").get<double>()
            );

            std::optional<double> targetPrice = std::nullopt;
            if (body.contains("target_price") && !body["target_price"].is_null()) {
                targetPrice = body["target_price"].get<double>();
            }

            TradeIntent intent(
                sideFromString(sideStr),
                instrument,
                currentPrice,
                stopPrice,
                targetPrice
            );

            ChecklistResponse responses;
            if (body.contains("checklist_responses")) {
                for (auto& [key, value] : body["checklist_responses"].items()) {
                    responses[key] = value.get<bool>();
                }
            }

            TradeSubmissionResult result = pipeline_.submit(
                intent, account, tmpl, responses);

            if (result.accepted) {
                accountStore_.save(account);
            }

            res.set_content(submissionResultToJson(result).dump(),
                            "application/json");

        } catch (const json::type_error& e) {
            writeError(res, 400, std::string("Invalid field type: ") + e.what());
        } catch (const json::out_of_range& e) {
            writeError(res, 400, std::string("Missing required field: ") + e.what());
        } catch (const std::invalid_argument& e) {
            writeError(res, 400, std::string("Invalid field value: ") + e.what());
        } catch (const std::exception& e) {
            writeError(res, 500, std::string("Internal server error: ") + e.what());
        }
    });

// ----- POST /trades -----
    server_.Post("/trades",
                 [this](const httplib::Request& req, httplib::Response& res) {
        // 1. Parse the request body.
        json body;
        try {
            body = json::parse(req.body);
        } catch (const json::parse_error&) {
            writeError(res, 400, "Invalid JSON in request body");
            return;
        }

        try {
            std::string accountId  = body.at("account_id").get<std::string>();
            std::string templateId = body.at("template_id").get<std::string>();
            std::string sideStr    = body.at("side").get<std::string>();
            double currentPrice    = body.at("current_price").get<double>();
            double stopPrice       = body.at("stop_price").get<double>();

            // NOTE: client also sends "size", but for now the pipeline still
            // computes the position size from risk %. We'll wire user-specified
            // size through the pipeline in a follow-up change.

            auto accountOpt = accountStore_.load(accountId);
            if (!accountOpt) {
                writeError(res, 404, "Account not found", "account_id", accountId);
                return;
            }
            Account account = *accountOpt;

            auto templateOpt = templateStore_.load(templateId);
            if (!templateOpt) {
                writeError(res, 404, "Checklist template not found",
                           "template_id", templateId);
                return;
            }
            ChecklistTemplate tmpl = *templateOpt;

            json instrumentJson = body.at("instrument");
            Instrument instrument(
                instrumentJson.at("symbol").get<std::string>(),
                instrumentJson.at("dollar_per_point").get<double>(),
                instrumentJson.at("tick_size").get<double>()
            );

            std::optional<double> targetPrice = std::nullopt;
            if (body.contains("target_price") && !body["target_price"].is_null()) {
                targetPrice = body["target_price"].get<double>();
            }

            TradeIntent intent(
                sideFromString(sideStr),
                instrument,
                currentPrice,      // entry = current chart price (market order)
                stopPrice,
                targetPrice
            );

            ChecklistResponse responses;
            if (body.contains("checklist_responses")) {
                for (auto& [key, value] : body["checklist_responses"].items()) {
                    responses[key] = value.get<bool>();
                }
            }

            TradeSubmissionResult result = pipeline_.submit(
                intent, account, tmpl, responses);

            if (result.accepted) {
                accountStore_.save(account);
            }

            res.set_content(submissionResultToJson(result).dump(),
                            "application/json");

        } catch (const json::type_error& e) {
            writeError(res, 400, std::string("Invalid field type: ") + e.what());
        } catch (const json::out_of_range& e) {
            writeError(res, 400, std::string("Missing required field: ") + e.what());
        } catch (const std::invalid_argument& e) {
            writeError(res, 400, std::string("Invalid field value: ") + e.what());
        } catch (const std::exception& e) {
            writeError(res, 500, std::string("Internal server error: ") + e.what());
        }
    });

    // ----- GET /trades?limit=N -----
    server_.Get("/trades",
                [this](const httplib::Request& req, httplib::Response& res) {
        int limit = 10;
        if (req.has_param("limit")) {
            try {
                limit = std::stoi(req.get_param_value("limit"));
            } catch (const std::exception&) {
                writeError(res, 400, "Invalid limit parameter");
                return;
            }
        }

        auto trades = journal_.recentTrades(limit);

        json tradeArray = json::array();
        for (const auto& order : trades) {
            tradeArray.push_back(orderToJson(order));
        }

        json response = {{"trades", tradeArray}};
        res.set_content(response.dump(), "application/json");
    });
}

void HttpServer::listen(const std::string& host, int port) {
    std::cout << "Server listening on http://" << host << ":" << port << "\n";
    server_.listen(host, port);
}