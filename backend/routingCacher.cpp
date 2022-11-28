#include "routingCacher.h"

#include <boost/json.hpp>
// #include <boost/json/value.hpp>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "boardingStatistics.h"
#include "gtfsTypes.h"

const auto haha_it_broke = EXIT_FAILURE;

namespace routingCacher {
using namespace routing;

/*
 * {
 *      stopId [string]: {
 *          time: [i32],
 *          incoming: {
 *              from: stopId [u64],
 *              fromStr: string,
 *              trip: tripId [u64],
 *              tripStr: string
 *          }
 *      }
 * }
 */

std::string toJson(std::unordered_map<StopId, StopState> map) {
    boost::json::object finalObj{};
    finalObj.reserve(map.size());

    std::for_each(map.begin(), map.end(), [&finalObj](const std::pair<StopId, StopState>& entry) {
        std::vector<boost::json::value> incomingTrips;
        incomingTrips.reserve(entry.second.incoming.size());

        std::transform(entry.second.incoming.begin(), entry.second.incoming.end(), std::back_inserter(incomingTrips),
                       [](IncomingTrip trip) {
                           boost::json::value v = {{"from", trip.from->stopId},
                                                   {"fromStr", std::to_string(trip.from->stopId)},
                                                   {"trip", trip.tripId},
                                                   {"tripStr", std::to_string(trip.tripId)}};
                           return v;
                       });

        boost::json::value val = {{"time", entry.second.travelTime}, {"incoming", incomingTrips}};
        finalObj.emplace(std::to_string(entry.first), val);
    });

    return serialize(finalObj);
}

void printFile(const std::string& str, const std::string& path) {
    std::fstream file;
    file.open(path, std::ios_base::out);
    if (!file.is_open()) {
        std::cout << "[ERROR!] Unable to open output file " << path << std::endl;
        return;
    }
    file << str;
    file.close();
}

void toFile(std::unordered_map<StopId, StopState> map, const std::string& path) {
    printFile(toJson(std::move(map)), path);
}

/* {
 *      stopId [string]: {
 *          time: [i32],
 *          incoming: {
 *              from: stopId [u64],
 *              fromStr: string,
 *              trip: tripId [u64],
 *              tripStr: string
 *          }
 *      }
 * }
 */

std::unordered_map<StopId, ParsedStopState> fromJson(const std::string& json) {
    std::unordered_map<StopId, ParsedStopState> res;
    auto parsed = boost::json::parse(json);
    auto rootObj = parsed.as_object();

    std::transform(rootObj.begin(), rootObj.end(), std::inserter(res, res.end()), [](boost::json::key_value_pair pair) {
        uint64_t stopId = std::stoull(pair.key());
        auto time = pair.value().at("time").to_number<int32_t>();
        auto trips = pair.value().at("incoming").as_array();

        std::vector<ParsedIncomingTrip> incomings;
        incomings.reserve(trips.size());
        std::transform(trips.begin(), trips.end(), std::back_inserter(incomings), [](boost::json::value val) {
            return ParsedIncomingTrip{val.at("from").to_number<uint64_t>(), val.at("trip").to_number<uint64_t>()};
        });

        ParsedStopState state{time, incomings};

        return std::make_pair(stopId, state);
    });

    return res;
}

std::unordered_map<StopId, ParsedStopState> fromFile(const std::string& path) {
    std::ifstream file;
    file.open(path);

    if (!file.is_open()) {
        std::cerr << "[ERROR!] Could not open file " << path << std::endl;
        return {};
    }

    std::string str(std::istreambuf_iterator<char>{file}, {});
    return fromJson(str);
}

std::unordered_map<StopId, ParsedStopState> toPSS(std::unordered_map<StopId, StopState> map) {
    std::unordered_map<StopId, ParsedStopState> res;
    res.reserve(map.size());

    std::transform(map.begin(), map.end(), std::inserter(res, res.end()),
                   [](const std::pair<StopId, StopState>& entry) {
                       std::vector<ParsedIncomingTrip> incomingTrips;
                       incomingTrips.reserve(entry.second.incoming.size());

                       std::transform(entry.second.incoming.begin(), entry.second.incoming.end(),
                                      std::back_inserter(incomingTrips), [](IncomingTrip trip) {
                                          return ParsedIncomingTrip{trip.from->stopId, trip.tripId};
                                      });

                       return std::make_pair(entry.first, ParsedStopState{entry.second.travelTime, incomingTrips});
                   });

    return res;
}

void test() {
    std::cout << "[TEST] Testing routingCacher on sample input" << std::endl;

    std::string str =
        "{\"10\": {"
        "\"time\": 20,"
        "\"incoming\": [{"
        "\"from\": 30,"
        "\"trip\": 40"
        "}]"
        "},"
        "\"100\":{"
        "\"time\":200,"
        "\"incoming\": [{"
        "\"from\": 300,"
        "\"trip\": 400"
        "},{"
        "\"from\": 333,"
        "\"trip\": 444"
        "}]"
        "}"
        "}";

    std::cout << "input:" << std::endl << str << std::endl << "parse:" << std::endl;

    auto res = fromJson(str);
    std::cout << "[" << std::endl;
    std::for_each(res.begin(), res.end(), [](std::pair<StopId, ParsedStopState> entry) {
        std::cout << "stop id: " << entry.first;
        std::cout << ", time: " << entry.second.travelTime << std::endl;
        std::cout << "incoming [" << entry.second.incoming.size() << "]: {" << std::endl;
        for (auto e : entry.second.incoming) {
            std::cout << "from: " << e.from << ", trip: " << e.tripId << std::endl;
        }
        std::cout << "}" << std::endl;
    });
    std::cout << "]" << std::endl;

    auto tests = {9021014001360000, 9021014005870000, 9021014004470000, 9021014005970000, 9021014005690000,
                  9021014006300000, 9021014001160000, 9021014003180000, 9021014002470000, 9021014004945000,
                  9021014004380000, 9021014001950000, 9021014001760000, 9021014004090000, 9021014002130000,
                  9021014007300000, 9021014006242000, 9021014002850000, 9021014001960000, 9021014001970000,
                  9021014005280000, 9021014004830000, 9021014003980000, 9021014008000000, 9021014019110000,
                  9021014003160000, 9021014001035000};

    std::cout << std::endl << "[TEST] Testing with Timetable and routing algorithm... loading timetable" << std::endl;
    auto startLoad = std::chrono::high_resolution_clock::now();
    routing::Timetable timetable("data/raw");
    auto stopLoad = std::chrono::high_resolution_clock::now();
    routing::RoutingOptions routingOptions = {10 * 60 * 60, 20221118, 30 * 60, 5 * 60};
    auto loadDuration = duration_cast<std::chrono::milliseconds>(stopLoad - startLoad).count();
    std::cout << "[TEST] Load done in " << loadDuration << "ms " << std::endl;

    for (auto id : tests) {
        std::cout << "[TEST] testing id " << id << "...";

        auto start = std::chrono::high_resolution_clock::now();
        auto result = timetable.dijkstra(9021014001360000, routingOptions);
        auto stopDijkstra = std::chrono::high_resolution_clock::now();

        auto normalized_map = routingCacher::toPSS(result);
        auto serialized = routingCacher::toJson(result);
        auto reparsed = routingCacher::fromJson(serialized);
        auto stopFinal = std::chrono::high_resolution_clock::now();

        auto dijDur = duration_cast<std::chrono::microseconds>(stopDijkstra - start).count();
        auto parseDur = duration_cast<std::chrono::microseconds>(stopFinal - stopDijkstra).count();

        auto success = normalized_map == reparsed;

        std::cout << (success ? "[SUCCESS]" : "[FAILURE]");
        std::cout << " [DIJ=" << dijDur << "µs] [PARSE=" << parseDur << "µs]" << std::endl;
    }

    boarding::load("data/raw/boarding_statistics.txt");

    std::cout << "[TEST] Running Dijkstras for the most important " << boarding::getStats().size() << " stops"
              << std::endl;
    std::unordered_map<StopId, std::string> allResults;

    auto djSt = std::chrono::high_resolution_clock::now();
    for (const auto& [key, val] : boarding::getStats()) {
        auto result = timetable.dijkstra(key, routingOptions);
        allResults.emplace(key, toJson(result));
    }
    auto djEnd = std::chrono::high_resolution_clock::now();

    auto parseDur = duration_cast<std::chrono::milliseconds>(djEnd - djSt).count();
    std::cout << "[TEST] Dijkstras + json conversion took " << parseDur << " ms, outputting to files..." << std::endl;
    for (const auto& [id, json] : allResults) {
        printFile(json, "data/idx/" + std::to_string(id) + "-graph.txt");
    }
    auto prEnd = std::chrono::high_resolution_clock::now();
    auto prDur = duration_cast<std::chrono::milliseconds>(prEnd - djEnd).count();
    std::cout << "[TEST] Saving graphs to files took " << prDur << " ms" << std::endl;

    uint64_t targetId = 9021014065802000;
    int runs = 50;
    std::cout << "[TEST] Comparing loading path to calculating it... " << runs << " runs" << std::endl;

    uint64_t totalEntriesLoad = 0;
    uint64_t totalDurLoad = 0;
    for (int i = 0; i < runs; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        auto map = fromFile("data/idx/" + std::to_string(targetId) + "-graph.txt");
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(end - start).count();
        totalEntriesLoad += map.size();
        totalDurLoad += duration;
    }

    uint64_t totalEntriesGenerate = 0;
    uint64_t totalDurGenerate = 0;
    for (int i = 0; i < runs; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        auto map = timetable.dijkstra(targetId, routingOptions);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(end - start).count();
        totalEntriesGenerate += map.size();
        totalDurGenerate += duration;
    }

    double durationLoad = static_cast<double>(totalDurLoad) / static_cast<double>(runs);
    double durationGenerate = static_cast<double>(totalDurGenerate) / static_cast<double>(runs);

    uint64_t entriesLoad = totalEntriesLoad / runs;
    uint64_t entriesGenerate = totalEntriesGenerate / runs;

    double improvement = durationLoad / durationGenerate;

    std::cout << "[TEST] [RUNNING DIJKSTRA VS LOADING GRAPH] [DIJKSTRA IS " << improvement << "x FASTER] [LOAD=";
    std::cout << durationLoad << "µs] [DIJKSTRA=" << durationGenerate << "µs] ";

    if (totalEntriesGenerate == totalEntriesLoad) {
        std::cout << "[BOTH HAD " << entriesLoad << " ENTRIES]" << std::endl;
    } else {
        std::cout << "[LOAD=" << entriesLoad << " ENTRIES] [GENERATE=" << entriesGenerate << " ENTRIES]" << std::endl;
    }
}

bool ParsedIncomingTrip::operator==(const ParsedIncomingTrip& rhs) const {
    return from == rhs.from && tripId == rhs.tripId;
}

bool ParsedIncomingTrip::operator!=(const ParsedIncomingTrip& rhs) const { return !(rhs == *this); }

bool ParsedStopState::operator==(const ParsedStopState& rhs) const {
    return travelTime == rhs.travelTime && incoming == rhs.incoming;
}

bool ParsedStopState::operator!=(const ParsedStopState& rhs) const { return !(rhs == *this); }
}  // namespace routingCacher
