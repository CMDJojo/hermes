#pragma once
#include <cstdint>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "gtfsTypes.h"

namespace routing {

const TripId WALK = 0;

class StopTime {
   public:
    TripId tripId;
    int32_t arrivalTime;
    int32_t departureTime;
    uint64_t stopId;
    int32_t stopSequence;

    explicit StopTime(int32_t departureTime) : departureTime(departureTime) {}

    StopTime(TripId tripId, int32_t arrivalTime, int32_t departureTime, uint64_t stopId, int32_t stopSequence)
        : tripId(tripId),
          arrivalTime(arrivalTime),
          departureTime(departureTime),
          stopId(stopId),
          stopSequence(stopSequence) {}
};

class StopNode;

struct IncomingTrip {
    StopNode* from{};
    TripId tripId{};

    IncomingTrip(StopNode* from, TripId tripId) : from(from), tripId(tripId) {}
};

struct Edge {
    StopNode* to;
    int32_t cost;
    TripId tripId;

    Edge(StopNode* to, int32_t weight, TripId tripId) : to(to), cost(weight), tripId(tripId) {}
};

struct Trip {
    ServiceId serviceId;
    std::vector<StopTime> stopTimes;
    int32_t directionId;
    RouteId routeId;
};

struct RoutingOptions {
    int32_t startTime;
    int32_t date;
    int32_t searchTime;
    int32_t minTransferTime;

    RoutingOptions(int32_t start_time, int32_t date, int32_t search_time, int32_t min_transfer_time)
        : startTime(start_time), date(date), searchTime(search_time), minTransferTime(min_transfer_time) {}
};

struct StopState {
    int32_t travelTime = std::numeric_limits<int32_t>::max();
    std::vector<IncomingTrip> incoming;
    bool visited = false;
};

class Timetable {
   public:
    std::unordered_map<StopId, StopNode> stops;
    std::unordered_map<StopId, std::vector<StopTime>> stopTimes;
    std::unordered_map<TripId, Trip> trips;
    std::unordered_map<ServiceId, std::set<int32_t>> calendarDates;
    std::unordered_map<RouteId, gtfs::Route> routes;

    Timetable(const std::string& gtfsPath);

    std::unordered_map<StopId, StopState> dijkstra(StopId start, const RoutingOptions& options);
};

class StopNode {
   public:
    StopId stopId{};
    std::string name;
    float lat;
    float lon;
    std::vector<Edge> transfers;

    std::vector<Edge> getEdges(Timetable& timetable, const RoutingOptions& options, StopState* state);
};

}  // namespace routing