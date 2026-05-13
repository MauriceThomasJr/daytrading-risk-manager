#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "storage/IAccountStore.h"
#include "storage/IChecklistTemplateStore.h"
#include "pipeline/TradePipeline.h"
#include <httplib.h>
#include <string>

class HttpServer {
public:
    HttpServer(IAccountStore& accountStore,
               IChecklistTemplateStore& templateStore,
               TradePipeline& pipeline);

    void listen(const std::string& host, int port);

private:
    void registerRoutes();

    httplib::Server server_;
    IAccountStore& accountStore_;
    IChecklistTemplateStore& templateStore_;
    TradePipeline& pipeline_;
};

#endif