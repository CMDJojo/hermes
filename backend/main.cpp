#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <vector>

#include "routing.h"

using namespace routing;

const routing::RoutingOptions routingOptions = {10 * 60 * 60, 20221110, 30 * 60, 5 * 50};

std::vector<int32_t> extractPath(Timetable& timetable, StopNode* stopNode) {
    if (stopNode->incoming.empty()) return {};
    StopNode* current = stopNode;
    TripId currentTrip = current->incoming.front().tripId;
    std::vector<int32_t> legs;
    while (!current->incoming.empty()) {
        auto from = current->incoming[0];
        for (auto node : current->incoming) {
            if (node.tripId == currentTrip) {
                from = node;
                break;
            }
        }
        currentTrip = from.tripId;
        int32_t line = (timetable.trips[currentTrip].routeId / 100000) % 100;
        legs.push_back(line);
        current = from.from;
    }
    std::reverse(legs.begin(), legs.end());
    return legs;
}

void printPath(Timetable& timetable, StopId stopId) {
    auto path = extractPath(timetable, &timetable.stops[stopId]);
    std::stringstream result;
    std::copy(path.begin(), path.end(), std::ostream_iterator<int>(result, ", "));

    std::cout << result.str() << std::endl;
}

int main() {
    routing::Timetable timetable("data/raw");

    auto start = std::chrono::high_resolution_clock::now();
    timetable.dijkstra(&timetable.stops[9022014001360000], routingOptions);
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() << " μs" << std::endl;

    std::cout << "Berghöjdsgatan: " << timetable.stops[9022014001360000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014001360000);
    std::cout << "Skogome: " << timetable.stops[9022014005870000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014005870000);
    std::cout << "Lillhagsparken Norra: " << timetable.stops[9022014004470000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014004470000);
    std::cout << "Skälltropsvägen: " << timetable.stops[9022014005970000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014005970000);
    std::cout << "Sankt Jörgens Park: " << timetable.stops[9022014005690000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014005690000);
    std::cout << "Stora Arödsgatan: " << timetable.stops[9022014006300000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014006300000);
    std::cout << "Aröds Äng: " << timetable.stops[9022014001160000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014001160000);
    std::cout << "Hjalmar Brantingsplatsen: " << timetable.stops[9022014003180000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014003180000);
    std::cout << "Frihamnen: " << timetable.stops[9022014002470000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014002470000);
    std::cout << "Nordstan: " << timetable.stops[9022014004945000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014004945000);
    std::cout << "Lilla Bommen: " << timetable.stops[9022014004380000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014004380000);
    std::cout << "Centralstationen: " << timetable.stops[9022014001950000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014001950000);
    std::cout << "Brunnsparken: " << timetable.stops[9022014001760000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014001760000);
    std::cout << "Kungsportsplatsen: " << timetable.stops[9022014004090000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014004090000);
    std::cout << "Domkyrkan: " << timetable.stops[9022014002130000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014002130000);
    std::cout << "Vasaplatsen: " << timetable.stops[9022014007300000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014007300000);
    std::cout << "Stenpiren: " << timetable.stops[9022014006242000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014006242000);
    std::cout << "Grönsakstorget: " << timetable.stops[9022014002850000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014002850000);
    std::cout << "Chalmers: " << timetable.stops[9022014001960000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014001960000);
    std::cout << "Chalmers Tvärgatan: " << timetable.stops[9022014001970000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014001970000);
    std::cout << "Pilbågsgatan: " << timetable.stops[9022014005280000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014005280000);
    std::cout << "Mossen: " << timetable.stops[9022014004830000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014004830000);
    std::cout << "Korsvägen: " << timetable.stops[9022014003980000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014003980000);
    std::cout << "Göteborg C: " << timetable.stops[9022014008000000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014008000000);
    std::cout << "Kungsbacka station: " << timetable.stops[9022014019110000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014019110000);
    std::cout << "Hinnebäcksgatan: " << timetable.stops[9022014003160000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014003160000);
    std::cout << "Donsö: " << timetable.stops[9022014001035000].travelTime / 60 << std::endl;
    printPath(timetable, 9022014001035000);

    return 0;
}