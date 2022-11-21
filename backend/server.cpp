#include <boost/json/src.hpp>

#include "routing.h"
#include "webServer/webServer.h"

const auto address = net::ip::make_address("0.0.0.0");
const auto port = static_cast<unsigned short>(8080);
const auto doc_root = std::make_shared<std::string>(".");
const auto threads = 1;

int main() {
    routing::Timetable timetable("data/raw");

    get("/", [](auto context) { return "Hello World!"; });

    get("/stops", [&timetable](auto context) {
        context.response.set(http::field::content_type, "application/geo+json");

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

    std::cout << "Starting server" << std::endl;

    startServer(address, port, doc_root, threads);
}
