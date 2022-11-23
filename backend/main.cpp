#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <vector>
#include <boost/json/src.hpp>

#include "routing.h"
#import "routingCacher.h"

using namespace routing;

const routing::RoutingOptions routingOptions = {10 * 60 * 60, 20221118, 30 * 60, 5 * 60};

std::vector<IncomingTrip> extractPath(Timetable& timetable, StopId stopId,
                                      std::unordered_map<StopId, StopState>& graph) {
    if (graph[stopId].incoming.empty()) return {};
    StopState* current = &graph.at(stopId);
    TripId currentTrip = current->incoming.front().tripId;
    std::vector<IncomingTrip> legs;
    while (!current->incoming.empty()) {
        IncomingTrip from = current->incoming[0];
        for (const IncomingTrip& node : current->incoming) {
            if (node.tripId == currentTrip) {
                from = node;
                break;
            }
        }
        currentTrip = from.tripId;
        legs.push_back(from);
        current = &graph[from.from->stopId];
    }
    std::reverse(legs.begin(), legs.end());
    return legs;
}

void printPath(Timetable& timetable, StopId stopId, std::unordered_map<StopId, StopState>& graph) {
    auto path = extractPath(timetable, stopId, graph);
    std::stringstream result;

    for (const auto& leg : path) {
        RouteId currentRouteId = timetable.trips[leg.tripId].routeId;
        result << (leg.tripId == WALK ? "Walk" : timetable.routes[currentRouteId].routeShortName) << ", ";
    }

    std::cout << timetable.stops[stopId].name << ": " << graph[stopId].travelTime / 60 << " min" << std::endl;
    std::cout << result.str() << std::endl;
}

int main() {
    routing::Timetable timetable("../../backend/data/raw");

    auto start = std::chrono::high_resolution_clock::now();
    auto result = timetable.dijkstra(9021014001360000, routingOptions);
    auto stop = std::chrono::high_resolution_clock::now();

    auto normalized_map = routingCacher::toPSS(result);
    auto serialized = routingCacher::toJson(result);
    std::cout << serialized << std::endl;
    auto reparsed = routingCacher::fromJson(serialized);
    std::cout << (normalized_map == reparsed ? "EQUAL" : "NOT EQUAL") << std::endl;
    exit(0);

    auto duration = duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() << " μs" << std::endl;

    printPath(timetable, 9021014001360000, result);
    printPath(timetable, 9021014005870000, result);
    printPath(timetable, 9021014004470000, result);
    printPath(timetable, 9021014005970000, result);
    printPath(timetable, 9021014005690000, result);
    printPath(timetable, 9021014006300000, result);
    printPath(timetable, 9021014001160000, result);
    printPath(timetable, 9021014003180000, result);
    printPath(timetable, 9021014002470000, result);
    printPath(timetable, 9021014004945000, result);
    printPath(timetable, 9021014004380000, result);
    printPath(timetable, 9021014001950000, result);
    printPath(timetable, 9021014001760000, result);
    printPath(timetable, 9021014004090000, result);
    printPath(timetable, 9021014002130000, result);
    printPath(timetable, 9021014007300000, result);
    printPath(timetable, 9021014006242000, result);
    printPath(timetable, 9021014002850000, result);
    printPath(timetable, 9021014001960000, result);
    printPath(timetable, 9021014001970000, result);
    printPath(timetable, 9021014005280000, result);
    printPath(timetable, 9021014004830000, result);
    printPath(timetable, 9021014003980000, result);
    printPath(timetable, 9021014008000000, result);
    printPath(timetable, 9021014019110000, result);
    printPath(timetable, 9021014003160000, result);
    printPath(timetable, 9021014001035000, result);

    return 0;
}
