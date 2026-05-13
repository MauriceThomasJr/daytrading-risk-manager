// src/main_http.cpp
//
// HTTP server entry point. Exposes the trade pipeline over REST.

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

int main() {
    httplib::Server server;

    server.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json response = {
            {"status", "ok"},
            {"service", "daytrading-risk-manager"}
        };
        res.set_content(response.dump(), "application/json");
    });

    const std::string host = "0.0.0.0";
    const int port = 8080;

    std::cout << "Server listening on http://" << host << ":" << port << "\n";
    std::cout << "Try: curl http://localhost:" << port << "/health\n";

    server.listen(host, port);
    return 0;
}