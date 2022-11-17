#pragma once

#include "routeTypes.h"

void startServer(const net::ip::address &address, unsigned short port, const std::shared_ptr<std::string> &doc_root, int threads);
void startServer(int argc, char *argv[]);
