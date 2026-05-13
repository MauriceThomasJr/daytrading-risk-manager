// src/main_http.cpp

#include "http/HttpServer.h"
#include "storage/SqliteAccountStore.h"
#include "storage/SqliteChecklistTemplateStore.h"
#include "journal/SqliteTradeJournal.h"
#include "broker/MockBrokerAdapter.h"
#include "risk/RiskManager.h"
#include "checklist/ChecklistGate.h"
#include "checklist/ChecklistItem.h"
#include "domain/Account.h"
#include "pipeline/TradePipeline.h"

#include <iostream>

namespace {
    void seedAccount(IAccountStore& store,
                     const std::string& id, double balance) {
        if (!store.exists(id)) {
            store.save(Account(id, balance));
        }
    }

    void seedTemplate(IChecklistTemplateStore& store) {
        if (store.exists("daily-es")) return;

        ChecklistTemplate tmpl;
        tmpl.addItem(ChecklistItem("volume", "Is there volume confirmation?"));
        tmpl.addItem(ChecklistItem("htf",    "Did you check the higher timeframe?"));
        tmpl.addItem(ChecklistItem("plan",   "Is this trade in your written plan?"));
        store.save("daily-es", tmpl);
    }
}

int main() {
    // Persistence layer.
    SqliteAccountStore accountStore("accounts.db");
    SqliteChecklistTemplateStore templateStore("accounts.db");
    SqliteTradeJournal journal("trades.db");

    // In-memory pieces.
    MockBrokerAdapter broker;
    RiskManager rules(0.01, 0.03, 5);  // 1% per trade, 3% daily loss, 5/day
    ChecklistGate gate;

    TradePipeline pipeline(gate, rules, journal, broker);

    // Seed demo data on first run.
    seedAccount(accountStore, "alice", 50000.0);
    seedAccount(accountStore, "bob",   100000.0);
    seedTemplate(templateStore);

    // Start the server.
    HttpServer server(accountStore, templateStore, pipeline);
    server.listen("0.0.0.0", 8080);

    return 0;
}