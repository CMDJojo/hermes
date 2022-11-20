#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

using TripId = uint64_t;
using StopId = uint64_t;
using ServiceId = int32_t;

namespace routing {

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
    uint64_t routeId;
};

struct RoutingOptions {
    int32_t startTime;
    int32_t date;
    int32_t searchTime;
    int32_t minTransferTime;
    
    RoutingOptions(int32_t start_time, int32_t date, int32_t search_time, int32_t min_transfer_time)
        : startTime(start_time), date(date), searchTime(search_time), minTransferTime(min_transfer_time) {}
};

class Timetable {
   public:
    std::unordered_map<StopId, StopNode> stops;
    std::unordered_map<StopId, std::vector<StopTime>> stopTimes;
    std::unordered_map<TripId, Trip> trips;
    std::unordered_map<ServiceId, int32_t> calendarDates;

    Timetable(const std::string& gtfsPath);

    void dijkstra(StopNode* start, const RoutingOptions& options);
};

class StopNode {
   public:
    StopId stopId{};
    int32_t travelTime{};
    std::vector<IncomingTrip> incoming;
    int32_t minTransferTime{};
    bool visited{};

    std::vector<Edge> getEdges(Timetable& graph, const RoutingOptions& options);
};

}  // namespace routing