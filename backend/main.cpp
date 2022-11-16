#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <fstream>
#include <functional>
#include <iterator>
#include <chrono>

using TripId = uint64_t;
using StopId = uint64_t;
using ServiceId = int32_t;

int32_t startTime = 10 * 60 * 60; // 10:00
int32_t date = 20221110;
int32_t searchTime = 30 * 60;
int32_t minTransferTime = 5 * 60;

class StopNode;

struct TripFrom {
    StopNode* from;
    TripId tripId;

    TripFrom(StopNode* from, TripId tripId) : from(from), tripId(tripId) {}
};

class Edge {
public:
    StopNode* to;
    int32_t weight;
    TripId tripId;

    Edge(StopNode *to, int32_t weight, TripId tripId) : to(to), weight(weight), tripId(tripId) {}
};

class StopNode {
public:
    StopId stopId{};
    int32_t travelTime{};
    std::vector<TripFrom> from;
    int32_t minTransferTime{};
    bool visited{};

    std::vector<Edge> getEdges();
};

class StopTime {
public:
    TripId tripId;
    int32_t arrivalTime;
    int32_t departureTime;
    uint64_t stopId;
    int32_t stopSequence;

    explicit StopTime(int32_t departureTime) : departureTime(departureTime) {}

    StopTime(TripId tripId, int32_t arrivalTime, int32_t departureTime, uint64_t stopId, int32_t stopSequence) : tripId(
            tripId), arrivalTime(arrivalTime), departureTime(departureTime), stopId(stopId), stopSequence(
            stopSequence) {}
};

struct Trip {
public:
    ServiceId serviceId;
    std::vector<StopTime> stopTimes;
    int32_t directionId;
    uint64_t routeId;
};

class Graph {
public:
    std::unordered_map<StopId, StopNode> nodes;
    std::unordered_map<StopId, std::vector<StopTime>> stops;
    std::unordered_map<TripId, Trip> trips;
    std::unordered_map<ServiceId, int32_t> calendarDates;

    static int32_t parseTime(const std::string& str) {
        std::stringstream ss(str);
        int32_t h, m, s;
        char c;
        ss >> h >> c >> m >> c >> s;
        return h * 60 * 60 + m * 60 + s;
    }

    static void parseCsv(const std::string& filename, const std::function<void(std::vector<std::string>)>& parseLine) {
        std::ifstream filestream;
        filestream.open(filename);

        std::string line;
        getline(filestream, line);

        std::cout << line << std::endl;

        while (filestream.peek() != EOF) {
            if (getline(filestream, line)) {
                std::stringstream ss(line);
                std::vector<std::string> splits;
                while(ss.good()) {
                    std::string elem;
                    getline(ss, elem, ',');
                    splits.push_back(elem);
                }

                parseLine(splits);
            }
        }
        filestream.close();

        std::cout << "Klar" << std::endl;
    }

    void loadGtfs() {
        parseCsv("trips.txt", [this](auto splits) {
            Trip trip {
                    std::stoi(splits[1]),
                    {},
                    std::stoi(splits[4]),
                    std::stoul(splits[0])
            };

            TripId tripId = std::stoul(splits[2]);
            trips[tripId] = trip;
        });

        parseCsv("stop_times.txt", [this](auto splits) {
            StopId stopId = std::stoul(splits[3]);
            stopId = stopId - stopId % 1000;

            StopTime stopTime{
                    std::stoul(splits[0]),
                    parseTime(splits[1]),
                    parseTime(splits[2]),
                    stopId,
                    std::stoi(splits[4]),
            };

            stops[stopId].push_back(stopTime);
            trips[stopTime.tripId].stopTimes.push_back(stopTime);
        });

        for (auto& [stopId, stopTimes]: stops) {
            auto compare = [](StopTime a, StopTime b) { return a.departureTime < b.departureTime; };
            std::sort(stopTimes.begin(), stopTimes.end(), compare);
        }

        parseCsv("calendar_dates.txt", [this](auto splits){
            ServiceId serviceId = std::stoi(splits[0]);
            int32_t date = std::stoi(splits[1]);
            calendarDates[serviceId] = date;
        });

        parseCsv("stops.txt", [this](auto splits){
            StopId stopId = std::stoul(splits[0]);
            stopId = stopId - stopId % 1000;
            StopNode stopNode{stopId, 0, {}, true, minTransferTime, false};
            nodes[stopId] = stopNode;
        });
    }
};

Graph graph;

std::vector<int32_t> extractPath(StopNode* stopNode) {
    if (stopNode->from.empty()) return {};
    StopNode* current = stopNode;
    TripId currentTrip = current->from.front().tripId;
    std::vector<int32_t> legs;
    while (!current->from.empty()) {
        auto from = current->from[0];
        for (auto node : current->from) {
            if (node.tripId == currentTrip){
                from = node;
                break;
            }
        }
        currentTrip = from.tripId;
        int32_t line = (graph.trips[currentTrip].routeId / 100000) % 100;
        legs.push_back(line);
        current = from.from;
    }
    std::reverse(legs.begin(), legs.end());
    return legs;
}

std::vector<Edge> StopNode::getEdges() {
    auto stop = &graph.stops[stopId];
    auto compare = [](StopTime a, StopTime b) { return a.departureTime < b.departureTime; };

    std::vector<Edge> outgoingEdges;
    std::set<uint64_t> outgoingDirections;

    // Alternative journeys
    for (TripFrom trip : from) {
        auto &stopTimes = graph.trips[trip.tripId].stopTimes;
        auto iter = std::ranges::find_if(stopTimes,
                                         [this](StopTime stopTime) { return stopTime.stopId == stopId; });
        uint32_t stopSequence = std::distance(stopTimes.begin(), iter) + 1;

        // Skip if final stop
        if (stopSequence >= stopTimes.size()) continue;

        // Get next stop
        StopTime next = stopTimes[stopSequence];

        outgoingEdges.emplace_back(&graph.nodes[next.stopId], next.arrivalTime - startTime - travelTime,
                                   trip.tripId);
    }

    auto iter = std::lower_bound(stop->begin(), stop->end(), StopTime(startTime + travelTime + minTransferTime),
                                 compare);

    for (; iter < stop->end() && iter->departureTime < startTime + travelTime + searchTime; iter++) {
        Trip *trip = &graph.trips[iter->tripId];

        // Max one departure per line and direction
        uint64_t direction = trip->routeId + trip->directionId;
        if (outgoingDirections.contains(direction)) continue;
        outgoingDirections.insert(direction);

        // Check date for departure
        if (graph.calendarDates[trip->serviceId] == date) continue;

        // Skip if final stop
        if (iter->stopSequence >= trip->stopTimes.size()) continue;

        // Get next stop
        StopTime next = trip->stopTimes[iter->stopSequence];

        // Skip departure if the next stop is the stop that you came from
        if (!from.empty() && next.stopId == from[0].from->stopId) continue;

        outgoingEdges.emplace_back(&graph.nodes[next.stopId], next.arrivalTime - startTime - travelTime,
                                   iter->tripId);
    }
    return outgoingEdges;
}

void dijkstra(StopNode* start) {
    auto compare = [](StopNode *a, StopNode *b) { return a->travelTime > b->travelTime; };
    std::priority_queue<StopNode*, std::vector<StopNode*>, decltype(compare)> queue;

    for (auto& node : graph.nodes) {
        node.second.travelTime = std::numeric_limits<int32_t>::max();
        node.second.visited = false;
    }

    start->travelTime = 0;
    queue.push(start);

    while (!queue.empty()) {
        StopNode *node = queue.top();
        queue.pop();

        // Ignore duplicates
        if (node->visited) continue;
        node->visited = true;

        for (Edge &edge: node->getEdges()) {
            int32_t newTravelTime = node->travelTime + edge.weight;
            if (newTravelTime < edge.to->travelTime) {
                edge.to->travelTime = newTravelTime;
                edge.to->from.insert(edge.to->from.begin(), TripFrom(node, edge.tripId));
                queue.push(edge.to);
            } else if (newTravelTime < edge.to->travelTime + edge.to->minTransferTime) {
                // Alternative journey that may result in fewer transfers and therefore faster travel time.
                edge.to->from.emplace_back(node, edge.tripId);
            }
        }
    }
}

void printPath(StopId stopId) {
    auto path = extractPath(&graph.nodes[stopId]);
    std::stringstream result;
    std::copy(path.begin(), path.end(), std::ostream_iterator<int>(result, ", "));

    std::cout << result.str() << std::endl;
}

int main() {
    graph.loadGtfs();

    auto start = std::chrono::high_resolution_clock::now();
    dijkstra(&graph.nodes[9022014001360000]);
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << duration.count() << " μs" << std::endl;

    std::cout << "Berghöjdsgatan: " << graph.nodes[9022014001360000].travelTime / 60 << std::endl;
    printPath(9022014001360000);
    std::cout << "Skogome: " << graph.nodes[9022014005870000].travelTime / 60 << std::endl;
    printPath(9022014005870000);
    std::cout << "Lillhagsparken Norra: " << graph.nodes[9022014004470000].travelTime / 60 << std::endl;
    printPath(9022014004470000);
    std::cout << "Skälltropsvägen: " << graph.nodes[9022014005970000].travelTime / 60 << std::endl;
    printPath(9022014005970000);
    std::cout << "Sankt Jörgens Park: " << graph.nodes[9022014005690000].travelTime / 60 << std::endl;
    printPath(9022014005690000);
    std::cout << "Stora Arödsgatan: " << graph.nodes[9022014006300000].travelTime / 60 << std::endl;
    printPath(9022014006300000);
    std::cout << "Aröds Äng: " << graph.nodes[9022014001160000].travelTime / 60 << std::endl;
    printPath(9022014001160000);
    std::cout << "Hjalmar Brantingsplatsen: " << graph.nodes[9022014003180000].travelTime / 60 << std::endl;
    printPath(9022014003180000);
    std::cout << "Frihamnen: " << graph.nodes[9022014002470000].travelTime / 60 << std::endl;
    printPath(9022014002470000);
    std::cout << "Nordstan: " << graph.nodes[9022014004945000].travelTime / 60 << std::endl;
    printPath(9022014004945000);
    std::cout << "Lilla Bommen: " << graph.nodes[9022014004380000].travelTime / 60 << std::endl;
    printPath(9022014004380000);
    std::cout << "Centralstationen: " << graph.nodes[9022014001950000].travelTime / 60 << std::endl;
    printPath(9022014001950000);
    std::cout << "Brunnsparken: " << graph.nodes[9022014001760000].travelTime / 60 << std::endl;
    printPath(9022014001760000);
    std::cout << "Kungsportsplatsen: " << graph.nodes[9022014004090000].travelTime / 60 << std::endl;
    printPath(9022014004090000);
    std::cout << "Domkyrkan: " << graph.nodes[9022014002130000].travelTime / 60 << std::endl;
    printPath(9022014002130000);
    std::cout << "Vasaplatsen: " << graph.nodes[9022014007300000].travelTime / 60 << std::endl;
    printPath(9022014007300000);
    std::cout << "Stenpiren: " << graph.nodes[9022014006242000].travelTime / 60 << std::endl;
    printPath(9022014006242000);
    std::cout << "Grönsakstorget: " << graph.nodes[9022014002850000].travelTime / 60 << std::endl;
    printPath(9022014002850000);
    std::cout << "Chalmers: " << graph.nodes[9022014001960000].travelTime / 60 << std::endl;
    printPath(9022014001960000);
    std::cout << "Chalmers Tvärgatan: " << graph.nodes[9022014001970000].travelTime / 60 << std::endl;
    printPath(9022014001970000);
    std::cout << "Pilbågsgatan: " << graph.nodes[9022014005280000].travelTime / 60 << std::endl;
    printPath(9022014005280000);
    std::cout << "Mossen: " << graph.nodes[9022014004830000].travelTime / 60 << std::endl;
    printPath(9022014004830000);
    std::cout << "Korsvägen: " << graph.nodes[9022014003980000].travelTime / 60 << std::endl;
    printPath(9022014003980000);
    std::cout << "Göteborg C: " << graph.nodes[9022014008000000].travelTime / 60 << std::endl;
    printPath(9022014008000000);
    std::cout << "Kungsbacka station: " << graph.nodes[9022014019110000].travelTime / 60 << std::endl;
    printPath(9022014019110000);
    std::cout << "Hinnebäcksgatan: " << graph.nodes[9022014003160000].travelTime / 60 << std::endl;
    printPath(9022014003160000);
    std::cout << "Donsö: " << graph.nodes[9022014001035000].travelTime / 60 << std::endl;
    printPath(9022014001035000);
    return 0;
}
