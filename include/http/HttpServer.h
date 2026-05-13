#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "storage/IAccountStore.h"
#include "storage/IChecklistTemplateStore.h"
#include "pipeline/TradePipeline.h"
#include "journal/ITradeJournal.h"
#include <httplib.h>
#include <string>

class HttpServer {
public:
    HttpServer(IAccountStore& accountStore,
               IChecklistTemplateStore& templateStore,
               TradePipeline& pipeline,
               ITradeJournal& journal);

    void listen(const std::string& host, int port);

private:
    void registerRoutes();

    httplib::Server server_;
    IAccountStore& accountStore_;
    IChecklistTemplateStore& templateStore_;
    TradePipeline& pipeline_;
    ITradeJournal& journal_;
};

#endif