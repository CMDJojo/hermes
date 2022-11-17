#include "routeTypes.h"
#include <fstream>

std::map<std::string, std::function<void(const ContextManualStatic, const send_lambda&)>> staticGets;
std::vector<std::pair<std::regex, std::function<void(const ContextManualDynamic, const send_lambda&)>>> dynamicGets;

void get(const std::string& path, const std::function<std::string(ContextEasyStatic)>& function) {
    get<http::string_body>(path, [function](ContextManualStatic context) {
        http::response<http::string_body> res{http::status::ok, context.request.version()};
        res.keep_alive(context.request.keep_alive());
        res.set(http::field::content_type, "text/html");
        res.body() = function(ContextEasyStatic {context.request, res});
        res.prepare_payload();
        return res;
    });
}

void get(const std::regex& regex, const std::function<std::string(ContextEasyDynamic)>& function) {
    get<http::string_body>(regex, [function](auto context) {
        http::response<http::string_body> res{http::status::ok, context.request.version()};
        res.keep_alive(context.request.keep_alive());
        res.set(http::field::content_type, "text/html");
        res.body() = function(ContextEasyDynamic {context.request, res, context.match});
        res.prepare_payload();
        return res;
    });
}
