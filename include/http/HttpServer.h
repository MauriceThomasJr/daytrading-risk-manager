#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "storage/IAccountStore.h"
#include <httplib.h>
#include <string>

class HttpServer {
public:
    HttpServer(IAccountStore& accountStore);

    void listen(const std::string& host, int port);

private:
    void registerRoutes();

    httplib::Server server_;
    IAccountStore& accountStore_;
};

#endif