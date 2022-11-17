#pragma once
#include <map>
#include <string>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <regex>
#include "sendLambda.h"

namespace beast = boost::beast;
namespace http = beast::http;

struct ContextEasyStatic {
    const http::request<http::string_body>& request;
    http::response<http::string_body>& response;
};

struct ContextManualStatic {
    const http::request<http::string_body>& request;
};

struct ContextEasyDynamic {
    const http::request<http::string_body>& request;
    http::response<http::string_body>& response;
    std::smatch match;
};

struct ContextManualDynamic {
    const http::request<http::string_body>& request;
    std::smatch match;
};

extern std::map<std::string, std::function<void(const ContextManualStatic, const send_lambda&)>> staticGets;
extern std::vector<std::pair<std::regex, std::function<void(const ContextManualDynamic, const send_lambda&)>>> dynamicGets;


template <class Body, typename Context>
static auto addSend(const std::function<http::response<Body>(Context)>& function) {
    return [function](Context context, const send_lambda& send) {
        send(function(context));
    };
}

// ManualStatic
template <class Body>
void get(const std::string& path, std::function<http::response<Body>(const ContextManualStatic)> function) {
    staticGets[path] = std::move(addSend(function));
}

// EasyStatic
void get(const std::string& path, const std::function<std::string(const ContextEasyStatic)>& function);

// ManualDynamic
template <class Body>
void get(const std::regex& regex, std::function<http::response<Body>(const ContextManualDynamic)> function) {
    dynamicGets.emplace_back(regex, std::move(addSend(function)));
}

// EasyDynamic
void get(const std::regex& regex, const std::function<std::string(const ContextEasyDynamic)>& function);
