#pragma once

#include <cstdint>
#include <limits>
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
    int32_t stopSequence;

    IncomingTrip(StopNode* from, TripId trip_id, int32_t stop_sequence)
        : from(from), tripId(trip_id), stopSequence(stop_sequence) {}
};

struct Edge {
    StopNode* to;
    int32_t cost;
    TripId tripId;
    int32_t stopSequence{};

    Edge(StopNode* to, int32_t cost, TripId trip_id, int32_t stop_sequence)
        : to(to), cost(cost), tripId(trip_id), stopSequence(stop_sequence) {}
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
    bool overrideMinTransferTime;

    RoutingOptions(int32_t start_time, int32_t date, int32_t search_time, int32_t min_transfer_time = 5 * 60,
                   bool override_min_transfer_time = false)
        : startTime(start_time),
          date(date),
          searchTime(search_time),
          minTransferTime(min_transfer_time),
          overrideMinTransferTime(override_min_transfer_time) {}
};

struct StopState {
    int32_t travelTime = std::numeric_limits<int32_t>::max();
    std::vector<IncomingTrip> incoming;
    bool visited = false;
    bool revisit = false;
};

class Timetable {
   public:
    std::unordered_map<StopId, StopNode> stops;
    std::unordered_map<StopId, std::vector<StopTime>> stopTimes;
    std::unordered_map<TripId, Trip> trips;
    std::unordered_map<ServiceId, std::set<int32_t>> calendarDates;
    std::unordered_map<RouteId, gtfs::Route> routes;

    explicit Timetable(const std::string& gtfsPath);

    std::unordered_map<StopId, StopState> dijkstra(StopId start, const RoutingOptions& options);
};

class StopNode {
   public:
    StopId stopId{};
    std::string name;
    float lat{};
    float lon{};
    std::unordered_map<TripId, std::vector<TripId>> transfersType1;
    std::vector<Edge> transfersType2;
    int32_t minTransferTime = 5 * 60;

    StopNode() = default;
    StopNode(StopId stop_id, std::string name, float lat, float lon)
        : stopId(stop_id), name(std::move(name)), lat(lat), lon(lon) {}

    std::vector<Edge> getEdges(Timetable& timetable, const RoutingOptions& options, StopState* state);

   private:
    void handleTransferType1(Timetable& timetable, const RoutingOptions& options, const StopState* state,
                             std::vector<Edge>& outgoingEdges, const IncomingTrip& trip);
};

std::string prettyTravelTime(int32_t time);
}  // namespace routing
