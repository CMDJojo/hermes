#include <algorithm>
#include <boost/json/src.hpp>
#include <boost/url/src.hpp>
#include <filesystem>
#include <iostream>

#include "binarySearch.h"
#include "endToEndEvaluator.h"
#include "lineRegister.h"
#include "people.h"
#include "routing.h"
#include "routingCacher.h"
#include "webServer/webServer.h"

const auto address = net::ip::make_address("0.0.0.0");
const auto port = static_cast<unsigned short>(8080);
const auto doc_root = std::make_shared<std::string>(".");
const auto threads = 4;

std::vector<std::shared_ptr<routing::Timetable>> timetables;
std::vector<std::shared_ptr<Prox>> proxes;

using namespace boost::urls;

std::function<BinarySearch::ComparatorResult(const Person&)> constructClosurePerson(float distance) {
    return [=](const Person& p) {
        auto d = p.distanceToWork();
        if (d > distance) return BinarySearch::ComparatorResult::LT;
        if (d < distance) return BinarySearch::ComparatorResult::GT;
        return BinarySearch::ComparatorResult::EQ;
    };
}

std::function<BinarySearch::ComparatorResult(const E2EE::PersonPath&)> constructClosurePersonPath(float time) {
    return [=](const E2EE::PersonPath& p) {
        auto t = p.timeAtGoal;
        if (t > time) return BinarySearch::ComparatorResult::LT;
        if (t < time) return BinarySearch::ComparatorResult::GT;
        return BinarySearch::ComparatorResult::EQ;
    };
}

template <class Body>
params_view getParams(const http::request<Body>& request) {
    result<url_view> u = boost::urls::parse_origin_form(request.target());
    if (u.has_error()) return {};
    return u->params();
}

routing::RoutingOptions routingOptionsFromParams(const params_view& params) {
    routing::RoutingOptions options{8 * 60 * 60, 20221216, 60 * 60};

    if (params.contains("date")) {
        options.date = std::stoi((*params.find("date")).value);
    }

    if (params.contains("time")) {
        options.startTime = std::stoi((*params.find("time")).value);
    }

    if (params.contains("searchTime")) {
        options.searchTime = std::stoi((*params.find("searchTime")).value);
    }

    if (params.contains("minTransferTime")) {
        int32_t minTransferTime = std::stoi((*params.find("minTransferTime")).value);
        if (minTransferTime >= 0) {
            options.overrideMinTransferTime = true;
            options.minTransferTime = minTransferTime;
        }
    }

    return options;
}

int32_t timetableIdFromParams(const params_view& params) {
    if (params.contains("timetable")) {
        int32_t id = std::stoi((*params.find("timetable")).value);
        if (id >= 0 && id < timetables.size()) return id;
    }
    return 0;
}

routing::Timetable& timetableFromParams(const params_view& params) {
    return *timetables.at(timetableIdFromParams(params));
}

int main() {
    std::cout << "Starting server..." << std::endl;
    std::cout << "Loading timetables (1/6)" << std::endl;

    for (const auto& gtfsEntry : std::filesystem::directory_iterator("data/gtfs")) {
        if (!gtfsEntry.is_directory()) continue;

        std::cout << "Loading timetable from " << gtfsEntry.path() << "..." << std::endl;
        timetables.emplace_back(new routing::Timetable(gtfsEntry.path().string()));
        std::cout << timetables.back()->name << " is loaded" << std::endl;
    }

    // Order timetables by start date
    std::sort(timetables.begin(), timetables.end(),
              [](const auto& a, const auto& b) { return a->startDate.original > b->startDate.original; });

    if (timetables.empty()) {
        std::cout << "No timetable found in data/gtfs, loading timetable from data/raw instead..." << std::endl;
        timetables.emplace_back(new routing::Timetable("data/raw"));
    }

    std::cout << "Loading lineregister (2/6)" << std::endl;
    LineRegister lineRegister("data/raw/lineregister.json");

    std::cout << "Loading people data (3/6)" << std::endl;
    People people("data/raw/Ast_bost.txt");

    std::cout << "Loading prox (4/6)" << std::endl;
    for (const auto& timetable : timetables) proxes.emplace_back(new Prox(*timetable));

    std::cout << "Configuring routes (5/6)" << std::endl;
    get("/", [](auto context) { return "Hello World!"; });

    get((std::regex) "/graphFrom/(\\d+).*", [](auto context) {
        context.response.set(http::field::content_type, "application/json");
        context.response.set(http::field::access_control_allow_origin, "*");

        auto params = getParams(context.request);
        auto routingOptions = routingOptionsFromParams(params);
        auto& timetable = timetableFromParams(params);

        auto match = std::stoull(context.match[1].str());
        auto result = timetable.dijkstra(match, routingOptions);
        return routingCacher::toJson(result);
    });

    get((std::regex) "/timetables", [](auto context) {
        context.response.set(http::field::content_type, "application/json");
        context.response.set(http::field::access_control_allow_origin, "*");

        std::vector<boost::json::value> tables;
        
        for (size_t i = 0; i < timetables.size(); i++) {
            auto& timetable = timetables[i];
            boost::json::value table = {
                {"name", timetable->name},
                {"id", i},
                {"startDate", timetable->startDate.original},
                {"endDate", timetable->endDate.original},
            };
            tables.push_back(table);
        }
        boost::json::value jsonResponse = {{"timetables", tables}};
        return serialize(jsonResponse);
    });

    get((std::regex) "/travelTimeLayer/(\\d+).*", [](auto context) {
        context.response.set(http::field::content_type, "application/geo+json");
        context.response.set(http::field::access_control_allow_origin, "*");

        auto params = getParams(context.request);
        auto routingOptions = routingOptionsFromParams(params);
        auto& timetable = timetableFromParams(params);

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
                                    {"travelTime", routing::prettyTravelTime(state.travelTime - state.initialWaitTime)},
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

    get("/stops", [](auto context) {
        context.response.set(http::field::content_type, "application/geo+json");
        context.response.set(http::field::access_control_allow_origin, "*");
        
        auto params = getParams(context.request);
        auto& timetable = timetableFromParams(params);

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

    get((std::regex) "/travelTime/(\\d+).*", [&](auto context) {
        context.response.set(http::field::access_control_allow_origin, "*");
        context.response.set(http::field::content_type, "application/json");

        auto params = getParams(context.request);
        auto routingOptions = routingOptionsFromParams(params);

        int32_t timetableId = timetableIdFromParams(params);
        auto& timetable = *timetables.at(timetableId);

        auto stopId = std::stoull(context.match[1].str());
        auto& stop = timetable.stops[stopId];
        auto stopCoord = DMSCoord(stop.lat, stop.lon);

        E2EE::Options options = {
            stopId, 0.6, 500, 500, 500, E2EE::COLLECT_ALL & (~E2EE::COLLECT_EXTRACTED_PATHS), routingOptions};

        E2EE endToEndEval(people, timetable, *proxes.at(timetableId));
        E2EE::Stats stats = endToEndEval.evaluatePerformanceAtPoint(stopCoord.toMeter(), options);

        uint32_t medianTravelTime = 0;
        std::string medianTravelTimeFormatted = {};
        size_t time15min = 0;
        size_t time30min = 0;
        size_t time60min = 0;
        size_t time90min = 0;
        size_t time180min = 0;
        size_t timeMore = 0;

        if (stats.allPaths.size() != 0) {
            // Find the mean travel time (I assume that it is okay to mutate the stats object?)
            std::sort(stats.allPaths.begin(), stats.allPaths.end(),
                      [](auto const& a, auto const& b) { return a.timeAtGoal < b.timeAtGoal; });

            time15min =
                BinarySearch::binarySearch<E2EE::PersonPath>(stats.allPaths, constructClosurePersonPath(60 * 15)).index;
            time30min = BinarySearch::binarySearch<E2EE::PersonPath>(
                            stats.allPaths, constructClosurePersonPath(60 * 30), time15min, stats.allPaths.size())
                            .index;
            time60min = BinarySearch::binarySearch<E2EE::PersonPath>(
                            stats.allPaths, constructClosurePersonPath(60 * 60), time30min, stats.allPaths.size())
                            .index;
            time90min = BinarySearch::binarySearch<E2EE::PersonPath>(
                            stats.allPaths, constructClosurePersonPath(60 * 90), time60min, stats.allPaths.size())
                            .index;
            time180min = BinarySearch::binarySearch<E2EE::PersonPath>(
                             stats.allPaths, constructClosurePersonPath(180 * 90), time90min, stats.allPaths.size())
                             .index;
            timeMore = stats.allPaths.size() - time180min;

            medianTravelTime = stats.allPaths[stats.allPaths.size() / 2].timeAtGoal;
            medianTravelTimeFormatted = routing::prettyTravelTime(medianTravelTime);
        }

        //        if (stats.allPaths.size() != 0) {
        //            medianTravelTime = stats.allPaths[stats.allPaths.size() / 2].timeAtGoal;
        //            medianTravelTimeFormatted = routing::prettyTravelTime(medianTravelTime);
        //        }

        std::vector<boost::json::value> segments;
        std::vector<boost::json::value> walks;

        for (const auto& [segmentId, segment] : stats.shapeSegments) {
            std::vector<boost::json::value> lineString;

            boost::json::object properties = {
                {"from", segment.startStop},
                {"to", segment.endStop},
                {"passengerCount", segment.passengerCount},
            };

            if (segment.tripId == routing::WALK) {
                auto& start = timetable.stops[segment.startStop];
                auto& end = timetable.stops[segment.endStop];
                lineString = {{start.lon, start.lat}, {end.lon, end.lat}};

                boost::json::value feature = {
                    {"type", "Feature"},
                    {"properties", properties},
                    {"geometry", {{"type", "LineString"}, {"coordinates", lineString}}},
                };

                walks.push_back(feature);
            } else {
                routing::Trip& trip = timetable.trips[segment.tripId];
                gtfs::Route& route = timetable.routes[trip.routeId];

                properties["routeName"] = route.routeShortName;
                properties["headsign"] = trip.stopTimes.at(segment.stopSequence - 1).stopHeadsign;

                auto& line = lineRegister.lines[trip.routeId];
                properties["fgColor"] = line.fgColor;
                properties["bgColor"] = line.bgColor;

                auto& shape = timetable.shapes[trip.shapeId];

                auto start = &shape[segment.startIdx];
                auto end = &shape[segment.endIdx];

                std::transform(start, end, std::back_inserter(lineString),
                               [](const std::pair<double, DMSCoord>& point) {
                                   boost::json::value coord = {point.second.longitude, point.second.latitude};
                                   return coord;
                               });

                boost::json::value feature = {
                    {"type", "Feature"},
                    {"properties", properties},
                    {"geometry", {{"type", "LineString"}, {"coordinates", lineString}}},
                };

                segments.push_back(feature);
            }
        }

        boost::json::value linesGeoJson = {{"type", "FeatureCollection"}, {"features", segments}};
        boost::json::value walksGeoJson = {{"type", "FeatureCollection"}, {"features", walks}};

        std::vector<std::pair<StopId, int>> sortedPpl;
        for (auto& a : stats.optimalFirstStop) sortedPpl.emplace_back(a);
        std::sort(sortedPpl.begin(), sortedPpl.end(), [](auto a, auto b) { return a.second > b.second; });

        std::vector<boost::json::value> pplTravelFrom;

        std::transform(sortedPpl.begin(), sortedPpl.end(), std::back_inserter(pplTravelFrom), [&timetable](auto pair) {
            auto [stopID, numberOfPeople] = pair;
            auto name = timetable.stops.contains(stopID) ? (timetable.stops.at(stopID).name)
                                                         : "[ID:" + std::to_string(stopID) + "]";

            return boost::json::value{{"stopID", std::to_string(stopID)},
                                      {"stopName", stopID == routing::WALK ? "Walk" : name},
                                      {"numberOfPersons", numberOfPeople}};
        });

        boost::json::value response = {
            {"totalNrPeople", stats.personsWithinRange},
            {"peopleCanGoByBus", stats.personsCanGoWithBus},
            {"optimalNrPeople", stats.hasThisAsOptimal},
            {"interestingStopID", std::to_string(stats.interestingStop)},
            {"medianTravelTime", medianTravelTime},
            {"medianTravelTimeFormatted", medianTravelTimeFormatted},
            {"peopleTravelFrom", pplTravelFrom},
            {"travelTimeStats",
             {{{"name", "< 15 min"}, {"data", time15min}},
              {{"name", "15-30 min"}, {"data", time30min - time15min}},
              {{"name", "30-60 min"}, {"data", time60min - time30min}},
              {{"name", "60-90 min"}, {"data", time90min - time60min}},
              {{"name", "90-180 min"}, {"data", time180min - time90min}},
              {{"name", "> 180 min"}, {"data", timeMore}}}},
            {"lines", linesGeoJson},
            {"walks", walksGeoJson},
        };

        return serialize(response);
    });

    // Generate an info report for a given stop (basically what gets shown in the sidebar).
    get((std::regex) "/travelDistance/(\\d+)", [&people](auto context) {
        context.response.set(http::field::access_control_allow_origin, "*");
        context.response.set(http::field::content_type, "application/json");

        auto params = getParams(context.request);
        auto& timetable = timetableFromParams(params);

        auto stopId = std::stoull(context.match[1].str());
        auto& stop = timetable.stops[stopId];
        auto stopCoord = DMSCoord(stop.lat, stop.lon);

        // Find every person that lives close (500 meter) to this given stop
        auto nearbyPeopleRangeMeter = 500;
        auto peopleNearby = people.personsInCircle(stopCoord, nearbyPeopleRangeMeter);

        // FIXME: replace the avg distance with travel time when there is a dijkstra function
        //  that takes arbitrary coordinates and not just stops.

        if (peopleNearby.empty()) {
            boost::json::value info = {{"nrPeople", 0},
                                       {"peopleRange", nearbyPeopleRangeMeter},
                                       {"medianDistance", 0},
                                       {"distanceStats",
                                        {{{"name", "< 1 km"}, {"distance", 0}},
                                         {{"name", "1-5 km"}, {"data", 0}},
                                         {{"name", "5-10 km"}, {"data", 0}},
                                         {{"name", "10-50 km"}, {"data", 0}},
                                         {{"name", "> 50 km"}, {"data", 0}}}}};

            return serialize(info);
        }

        int32_t nrPeople = peopleNearby.size();

        peopleNearby.erase(std::remove_if(peopleNearby.begin(), peopleNearby.end(),
                                          [](const Person& p) { return p.work_coord == MeterCoord(0, 0); }),
                           peopleNearby.end());

        std::sort(peopleNearby.begin(), peopleNearby.end(),
                  [](const Person& p1, const Person& p2) { return p1.distanceToWork() < p2.distanceToWork(); });

        uint32_t pseudoMedian = static_cast<uint32_t>(peopleNearby.at(peopleNearby.size() / 2).distanceToWork());

        size_t distance1km = BinarySearch::binarySearch<Person>(peopleNearby, constructClosurePerson(1000)).index;
        size_t distance5km = BinarySearch::binarySearch<Person>(peopleNearby, constructClosurePerson(5000), distance1km,
                                                                peopleNearby.size())
                                 .index;
        size_t distance10km = BinarySearch::binarySearch<Person>(peopleNearby, constructClosurePerson(10000),
                                                                 distance5km, peopleNearby.size())
                                  .index;
        size_t distance50km = BinarySearch::binarySearch<Person>(peopleNearby, constructClosurePerson(50000),
                                                                 distance10km, peopleNearby.size())
                                  .index;
        size_t distanceMore = peopleNearby.size() - distance50km;

        boost::json::value info = {{"nrPeople", nrPeople},
                                   {"peopleRange", nearbyPeopleRangeMeter},
                                   {"medianDistance", pseudoMedian},
                                   {"distanceStats",
                                    {{{"name", "< 1 km"}, {"data", distance1km}},
                                     {{"name", "1-5 km"}, {"data", distance5km - distance1km}},
                                     {{"name", "5-10 km"}, {"data", distance10km - distance5km}},
                                     {{"name", "10-50 km"}, {"data", distance50km - distance10km}},
                                     {{"name", "> 50 km"}, {"data", distanceMore}}}}};

        return serialize(info);
    });

    get((std::regex) ".*", [](auto context) {
        context.response.result(http::status::not_found);
        return "Not found";
    });

    std::cout << "Starting server at " << address << ":" << port << " (6/6)" << std::endl;
    startServer(address, port, doc_root, threads);
}
