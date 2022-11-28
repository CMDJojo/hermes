#include <boost/json/src.hpp>
#include <algorithm>

#include "people.h"
#include "routing.h"
#include "routingCacher.h"
#include "webServer/webServer.h"

const auto address = net::ip::make_address("0.0.0.0");
const auto port = static_cast<unsigned short>(8080);
const auto doc_root = std::make_shared<std::string>(".");
const auto threads = 1;

int main() {
    std::cout << "Loading timetable" << std::endl;
    routing::Timetable timetable("data/raw");
    const routing::RoutingOptions routingOptions = {10 * 60 * 60, 20221118, 30 * 60, 5 * 60};

    std::cout << "Loading people data" << std::endl;
    People people("data/raw/Ast_bost.txt");

    get("/", [](auto context) { return "Hello World!"; });

    get((std::regex) "/graphFrom/(\\d+)", [&timetable, &routingOptions](auto context) {
        context.response.set(http::field::content_type, "application/json");
        context.response.set(http::field::access_control_allow_origin, "*");
        auto match = std::stoull(context.match[1].str());
        auto result = timetable.dijkstra(match, routingOptions);
        return routingCacher::toJson(result);
    });

    get((std::regex) "/travelTimeLayer/(\\d+)", [&timetable, &routingOptions](auto context) {
        context.response.set(http::field::content_type, "application/geo+json");
        context.response.set(http::field::access_control_allow_origin, "*");

        auto match = std::stoull(context.match[1].str());
        auto graph = timetable.dijkstra(match, routingOptions);

        std::vector<boost::json::value> stops;
        std::transform(graph.begin(), graph.end(), std::back_inserter(stops),
                       [&timetable](const std::pair<StopId, routing::StopState>& entry) {
                           routing::StopState state = entry.second;
                           routing::StopNode stop = timetable.stops[entry.first];

                           boost::json::value feature = {
                               {"type", "Feature"},
                               {"id", stop.stopId},
                               {"properties",
                                {
                                    {"name", stop.name},
                                    {"travelTime", routing::prettyTravelTime(state.travelTime)},
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

    // Generate an info report for a given stop (basically what gets shown in the sidebar).
    get((std::regex) "/stopInfo/(\\d+)", [&timetable, &people](auto context) {
        context.response.set(http::field::access_control_allow_origin, "*");
        context.response.set(http::field::content_type, "application/json");

        auto stopId = std::stoull(context.match[1].str());
        auto& stop = timetable.stops[stopId];
        auto stopCoord = DMSCoord(stop.lat, stop.lon);

        // Find every person that lives close (500 meter) to this given stop
        auto nearbyPeopleRangeMeter = 500;
        auto peopleNearby = people.personsInCircle(stopCoord, nearbyPeopleRangeMeter);

        // FIXME: replace the avg distance with travel time when there is a dijkstra function
        //  that takes arbitrary coordinates and not just stops.

        // Calculate distance stats
        uint32_t avgDistance = 0;
        uint32_t distance1km = 0;
        uint32_t distance5km = 0;
        uint32_t distance10km = 0;
        uint32_t distance50km = 0;
        uint32_t distanceMore = 0;



        if (peopleNearby.empty()) {
            boost::json::value info = {{"nrPeople", 0},
                                       {"peopleRange", nearbyPeopleRangeMeter},
                                       {"medianDistance", 0},
                                       {"distanceStats",
                                               {{{"name", "< 1 km"}, {"distance", 0}},
                                                            {{"name", "1-5 km"}, {"distance", 0}},
                                                            {{"name", "5-10 km"}, {"distance", 0}},
                                                            {{"name", "10-50 km"}, {"distance", 0}},
                                                            {{"name", "> 50 km"}, {"distance", 0}}}}};

            return serialize(info);
        }

        std::sort(peopleNearby.begin(), peopleNearby.end(), [](const Person& p1, const Person& p2) {
           return p1.distanceToWork() < p2.distanceToWork();
        });

        uint32_t pseudoMedian =  static_cast<uint32_t>(peopleNearby.at(peopleNearby.size() / 2).distanceToWork());

        for (Person& person : peopleNearby) {
            float distanceToWork = person.home_coord.distanceTo(person.work_coord);

            if (distanceToWork < 1'000) {
                distance1km += 1;
            } else if (distanceToWork < 5'000) {
                distance5km += 1;
            } else if (distanceToWork < 10'000) {
                distance10km += 1;
            } else if (distanceToWork < 50'000) {
                distance50km += 1;
            } else {
                distanceMore += 1;
            }
        }

        boost::json::value info = {{"nrPeople", peopleNearby.size()},
                                   {"peopleRange", nearbyPeopleRangeMeter},
                                   {"medianDistance", pseudoMedian},
                                   {"distanceStats",
                                    {{{"name", "< 1 km"}, {"distance", distance1km}},
                                     {{"name", "1-5 km"}, {"distance", distance5km}},
                                     {{"name", "5-10 km"}, {"distance", distance10km}},
                                     {{"name", "10-50 km"}, {"distance", distance50km}},
                                     {{"name", "> 50 km"}, {"distance", distanceMore}}}}};

        return serialize(info);
    });

    get((std::regex) ".*", [](auto context) { return "Not found"; });

    std::cout << "Starting server at " << address << ":" << port << std::endl;

    startServer(address, port, doc_root, threads);
}
