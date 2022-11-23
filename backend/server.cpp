#include <boost/json/src.hpp>

#include "routing.h"
#include "webServer/webServer.h"
#include "routingCacher.h"

const auto address = net::ip::make_address("0.0.0.0");
const auto port = static_cast<unsigned short>(8080);
const auto doc_root = std::make_shared<std::string>(".");
const auto threads = 1;

int main() {
    std::cout << "Loading timetable" << std::endl;
    routing::Timetable timetable("../data/raw");
    const routing::RoutingOptions routingOptions = {10 * 60 * 60, 20221118, 30 * 60, 5 * 60};

    get("/", [](auto context) { return "Hello World!"; });

    get((std::regex) "/graphFrom/(\\d+)", [&timetable, &routingOptions](auto context) {
        context.response.set(http::field::content_type, "application/json");
        context.response.set(http::field::access_control_allow_origin, "*");
        auto match = std::stoull(context.match[1].str());
        auto result = timetable.dijkstra(match, routingOptions);
        return routingCacher::toJson(result);
    });

    get("/stops", [&timetable](auto context) {
        context.response.set(http::field::content_type, "application/geo+json");
        context.response.set(http::field::access_control_allow_origin, "*");

        std::vector<boost::json::value> stops;
        std::transform(timetable.stops.begin(), timetable.stops.end(), std::back_inserter(stops),
                       [](const std::pair<StopId, routing::StopNode>& entry) {
                           routing::StopNode stop = entry.second;
                           boost::json::value feature = {
                               {"type", "Feature"},
                               {"id", stop.stopId},
                               {"properties",
                                {
                                    {"name", stop.name},
                                }},
                               {"geometry",
                                {
                                    {"type", "Point"},
                                    {"coordinates", {stop.lon, stop.lat}},
                                }},
                           };
                           return feature;
                       });
        
        boost::json::value geoJson = {{"type", "FeatureCollection"}, {"features", stops}};
        return serialize(geoJson);
    });

    get((std::regex) ".*", [](auto context) { return "Not found"; });

    std::cout << "Starting server at " << address << ":" << port << std::endl;

    startServer(address, port, doc_root, threads);
}
