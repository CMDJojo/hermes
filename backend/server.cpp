#include "webServer/webServer.h"

const auto address = net::ip::make_address("0.0.0.0");
const auto port = static_cast<unsigned short>(8080);
const auto doc_root = std::make_shared<std::string>(".");
const auto threads = 1;

int main() {
    get("/", [](auto context) {
        return "Hello World!";
    });

    get((std::regex) ".*", [](auto context) {
        return "Not found";
    });

    startServer(address, port, doc_root, threads);
}
