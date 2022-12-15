#include <algorithm>
#include <boost/json/src.hpp>
#include <boost/url/src.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>

#include "binarySearch.h"
#include "boardingStatistics.h"
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
        auto t = p.timeAtGoal - p.initialWaitTime;
        if (t > time) return BinarySearch::ComparatorResult::LT;
        if (t < time) return BinarySearch::ComparatorResult::GT;
        return BinarySearch::ComparatorResult::EQ;
    };
}

const std::string* getStopIcon(StopId stopId) {
    const static int32_t tramStops[] = {1050, 1200, 1450, 1620, 1690, 1745, 1850, 1900, 2150, 2170, 2200,
                                        2210, 2370, 2470, 2540, 2630, 2670, 2730, 2790, 3040, 3060, 3360,
                                        3620, 3880, 4320, 4370, 4527, 4700, 4730, 4780, 4810, 4870, 5110,
                                        5140, 5170, 5220, 5330, 5531, 5630, 5660, 5710, 5740, 5763, 6040,
                                        6260, 6570, 7150, 7172, 7200, 7270, 7280, 7320, 7370, 7750, 8590};

    const static int32_t trainStops[] = {
        2672,  4525,  6605,  8000,  12115, 12610, 12711, 13100, 13715, 14212, 14715, 15010, 15280, 15300, 16100,
        16200, 16300, 16410, 16611, 17115, 17117, 17130, 17165, 17210, 17280, 17510, 17610, 18116, 18410, 18530,
        19110, 19115, 19131, 19820, 21120, 21293, 21510, 22110, 22220, 22310, 25120, 25210, 26110, 26140, 26210,
        27306, 30001, 30145, 31010, 31328, 31331, 31332, 31385, 32010, 34001, 34123, 34124, 34125, 34126, 34254,
        34255, 34289, 35004, 37010, 37234, 37375, 37376, 37386, 40010, 40206, 40217, 40218, 41010, 41280, 41291,
        43111, 44801, 45500, 52602, 61900, 62717, 66021, 66105, 66134, 66135, 72010, 72017, 72021, 74000, 75000,
        75045, 75047, 75048, 75053, 75054, 78000, 78158, 79003, 80800, 80801, 80802, 81600, 82177, 82900, 82902,
        82903, 82904, 85044, 86500, 88000, 88002, 88009, 88016, 88020, 89020};

    const static int32_t boatStops[] = {
        1031,  1033,  1034,  1035,  1036,  1038,  1039,  1043,  1044,  1045,  1048,  1049,  1061,  1062,  1064,  1066,
        2239,  3895,  4381,  4420,  4493,  6030,  9620,  11213, 11310, 11430, 11616, 11617, 11710, 11750, 11810, 11850,
        11910, 14290, 14294, 14295, 14296, 14297, 14310, 14312, 14315, 14590, 14592, 14662, 15120, 15530, 15534, 15535,
        15536, 15537, 15546, 15547, 15548, 15690, 15696, 15721, 15725, 15746, 15807, 15932, 23112, 23509, 23518, 23542,
        25226, 25292, 25298, 25302, 26115, 26180, 26190, 26410, 26411, 26420, 26421, 26422, 26430};

    const static std::string icons[] = {"tram", "train", "boat"};

    int32_t extId = (stopId / 1000) % 1000000;

    if (std::binary_search(std::begin(tramStops), std::end(tramStops), extId)) return &icons[0];
    if (std::binary_search(std::begin(trainStops), std::end(trainStops), extId)) return &icons[1];
    if (std::binary_search(std::begin(boatStops), std::end(boatStops), extId)) return &icons[2];

    return nullptr;
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
    std::cout << "Loading timetables (1/7)" << std::endl;

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

    std::cout << "Loading lineregister (2/7)" << std::endl;
    LineRegister lineRegister("data/raw/lineregister.json");

    std::cout << "Loading people data (3/7)" << std::endl;
    People people("data/raw/Ast_bost.txt");

    std::cout << "Loading prox (4/7)" << std::endl;
    for (const auto& timetable : timetables) proxes.emplace_back(new Prox(*timetable));

    std::cout << "Loading boarding statistics (5/7)" << std::endl;
    boarding::load("data/raw/boarding_statistics.txt");

    std::cout << "Configuring routes (6/7)" << std::endl;
    
    get("/", [](auto context) {
        std::ifstream s("public/index.html");
        std::string str((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());
        return str;
    });

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

                           boost::json::object properties = {{"name", stop.name}};
                           const std::string* icon = getStopIcon(entry.first);
                           if (icon) properties["icon"] = *icon;

                           boost::json::value feature = {
                               {"type", "Feature"},
                               {"id", stop.stopId},
                               {"properties", properties},
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
        auto& stop = timetable.stops.at(stopId);
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
        float avgWaitTime = 0;
        std::string avgWaitTimeFormatted = {};

        if (stats.allPaths.size() != 0) {
            // Find the mean travel time (I assume that it is okay to mutate the stats object?)
            std::sort(stats.allPaths.begin(), stats.allPaths.end(), [](auto const& a, auto const& b) {
                return a.timeAtGoal - a.initialWaitTime < b.timeAtGoal - b.initialWaitTime;
            });

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

            auto& path = stats.allPaths[stats.allPaths.size() / 2];

            medianTravelTime = path.timeAtGoal - path.initialWaitTime;
            medianTravelTimeFormatted = routing::prettyTravelTime(medianTravelTime);

            uint64_t totalInitialWaitTime =
                std::accumulate(stats.allPaths.begin(), stats.allPaths.end(), 0,
                                [](uint64_t sum, const auto& path) { return sum + path.initialWaitTime; });
            avgWaitTime = totalInitialWaitTime / stats.allPaths.size();
            avgWaitTimeFormatted = routing::prettyTravelTime(avgWaitTime);
        }

        //        if (stats.allPaths.size() != 0) {
        //            medianTravelTime = stats.allPaths[stats.allPaths.size() / 2].timeAtGoal;
        //            medianTravelTimeFormatted = routing::prettyTravelTime(medianTravelTime);
        //        }

        std::vector<boost::json::value> segments;
        std::vector<boost::json::value> walks;

        std::vector<std::pair<StopId, float>> sortedTransfers;
        for (const auto& [transferStopId, count] : stats.transfers) {
            float percentage = (float)count / (float)stats.allPaths.size() * 100.0f;
            if (count <= 1 || percentage < 1) continue;
            sortedTransfers.emplace_back(transferStopId, percentage);
        }
        std::sort(sortedTransfers.begin(), sortedTransfers.end(), [](auto& a, auto& b) { return a.second > b.second; });

        std::vector<boost::json::value> transfers;
        std::transform(sortedTransfers.begin(), sortedTransfers.end(), std::back_inserter(transfers),
                       [&timetable](auto& pair) {
                           auto [stopID, percentage] = pair;
                           auto& name = timetable.stops.at(stopID).name;

                           return boost::json::value{
                               {"stopID", std::to_string(stopID)}, {"stopName", name}, {"percentage", percentage}};
                       });

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

            return boost::json::value{
                {"stopID", std::to_string(stopID)}, {"stopName", name}, {"numberOfPersons", numberOfPeople}};
        });

        double avgStopsFrom = 0;
        double numGoingFrom = 0;

        double avgStopsTo = 0;
        double numGoingTo = 0;

        std::for_each(stats.distNumberOfStartStops.begin(), stats.distNumberOfStartStops.end(),
                      [&avgStopsFrom, &numGoingFrom](auto pair) {
                          auto [noStops, noPpl] = pair;
                          avgStopsFrom += noStops * noPpl;
                          numGoingFrom += noPpl;
                      });
        if (numGoingFrom != 0) {
            avgStopsFrom /= numGoingFrom;
        }

        std::for_each(stats.distNumberOfEndStops.begin(), stats.distNumberOfEndStops.end(),
                      [&avgStopsTo, &numGoingTo](auto pair) {
                          auto [noStops, noPpl] = pair;
                          avgStopsTo += noStops * noPpl;
                          numGoingTo += noPpl;
                      });
        if (numGoingTo != 0) {
            avgStopsTo /= numGoingTo;
        }

        std::vector<boost::json::value> distStopsFrom;
        std::transform(stats.distNumberOfStartStops.begin(), stats.distNumberOfStartStops.end(),
                       std::back_inserter(distStopsFrom), [](auto pair) {
                           auto [noStops, noPpl] = pair;
                           return boost::json::value{{"name", std::to_string(noStops)}, {"data", noPpl}};
                       });

        std::vector<boost::json::value> distStopsTo;
        std::transform(stats.distNumberOfEndStops.begin(), stats.distNumberOfEndStops.end(),
                       std::back_inserter(distStopsTo), [](auto pair) {
                           auto [noStops, noPpl] = pair;
                           return boost::json::value{{"name", std::to_string(noStops)}, {"data", noPpl}};
                       });

        boost::json::value response = {
            {"totalNrPeople", stats.personsWithinRange},
            {"peopleCanGoByBus", stats.personsCanGoWithBus},
            {"optimalNrPeople", stats.hasThisAsOptimal},
            {"interestingStopID", std::to_string(stats.interestingStop)},
            {"medianTravelTime", medianTravelTime},
            {"medianTravelTimeFormatted", medianTravelTimeFormatted},
            {"avgWaitTime", avgWaitTime},
            {"avgWaitTimeFormatted", avgWaitTimeFormatted},
            {"numberOfTransfers", stats.numberOfTransfers},
            {"transfers", transfers},
            {"peopleTravelFrom", pplTravelFrom},
            {"avgStopsFrom", avgStopsFrom},
            {"avgStopsTo", avgStopsTo},
            {"distStopsFrom", distStopsFrom},
            {"distStopsTo", distStopsTo},
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
    get((std::regex) "/travelDistance/(\\d+).*", [&people](auto context) {
        context.response.set(http::field::access_control_allow_origin, "*");
        context.response.set(http::field::content_type, "application/json");

        auto params = getParams(context.request);
        auto& timetable = timetableFromParams(params);

        auto stopId = std::stoull(context.match[1].str());
        auto& stop = timetable.stops.at(stopId);
        auto stopCoord = DMSCoord(stop.lat, stop.lon);

        // Find every person that lives close (500 meter) to this given stop
        auto nearbyPeopleRangeMeter = 500;
        auto peopleNearby = people.personsInCircle(stopCoord, nearbyPeopleRangeMeter);

        int32_t nrPeople = peopleNearby.size();

        peopleNearby.erase(std::remove_if(peopleNearby.begin(), peopleNearby.end(),
                                          [](const Person& p) { return p.work_coord == MeterCoord(0, 0); }),
                           peopleNearby.end());

        size_t pseudoMedian{}, distance1km{}, distance5km{}, distance10km{}, distance50km{}, distanceMore{};

        if (!peopleNearby.empty()) {
            std::sort(peopleNearby.begin(), peopleNearby.end(),
                      [](const Person& p1, const Person& p2) { return p1.distanceToWork() < p2.distanceToWork(); });

            pseudoMedian = peopleNearby.at(peopleNearby.size() / 2).distanceToWork();

            distance1km = BinarySearch::binarySearch<Person>(peopleNearby, constructClosurePerson(1000)).index;
            distance5km = BinarySearch::binarySearch<Person>(peopleNearby, constructClosurePerson(5000), distance1km,
                                                             peopleNearby.size())
                              .index;
            distance10km = BinarySearch::binarySearch<Person>(peopleNearby, constructClosurePerson(10000), distance5km,
                                                              peopleNearby.size())
                               .index;
            distance50km = BinarySearch::binarySearch<Person>(peopleNearby, constructClosurePerson(50000), distance10km,
                                                              peopleNearby.size())
                               .index;
            distanceMore = peopleNearby.size() - distance50km;
        }

        boost::json::object info = {{"nrPeople", nrPeople},
                                    {"peopleRange", nearbyPeopleRangeMeter},
                                    {"medianDistance", pseudoMedian},
                                    {"minTransferTime", stop.minTransferTime},
                                    {"distanceStats",
                                     {{{"name", "< 1 km"}, {"data", distance1km}},
                                      {{"name", "1-5 km"}, {"data", distance5km - distance1km}},
                                      {{"name", "5-10 km"}, {"data", distance10km - distance5km}},
                                      {{"name", "10-50 km"}, {"data", distance50km - distance10km}},
                                      {{"name", "> 50 km"}, {"data", distanceMore}}}}};

        auto& boardingStats = boarding::getStats();
        if (boardingStats.contains(stopId)) info["boardings"] = boardingStats.at(stopId);

        return serialize(info);
    });

    get((std::regex) ".*", [](auto context) {
        context.response.result(http::status::not_found);
        return "Not found";
    });

    std::cout << "Starting server at " << address << ":" << port << " (7/7)" << std::endl;
    startServer(address, port, doc_root, threads);
}
