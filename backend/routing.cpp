#include "routing.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <set>
#include <vector>

#include "gtfsTypes.h"

using namespace routing;

std::unordered_map<StopId, StopState> Timetable::dijkstra(StopId start, const RoutingOptions& options) {
    std::unordered_map<StopId, StopState> state;

    auto compare = [](std::pair<StopNode*, StopState*> a, std::pair<StopNode*, StopState*> b) {
        return a.second->travelTime > b.second->travelTime;
    };
    std::priority_queue<std::pair<StopNode*, StopState*>, std::vector<std::pair<StopNode*, StopState*>>,
                        decltype(compare)>
        queue;

    state[start].travelTime = 0;

    queue.emplace(&stops[start], &state[start]);

    while (!queue.empty()) {
        auto [node, nodeState] = queue.top();
        queue.pop();

        // Ignore duplicates
        if (nodeState->visited) continue;
        nodeState->visited = true;

        for (Edge& edge : node->getEdges(*this, options, nodeState)) {
            int32_t newTravelTime = nodeState->travelTime + edge.cost;
            StopState& toState = state[edge.to->stopId];
            if (newTravelTime < toState.travelTime) {
                toState.travelTime = newTravelTime;
                toState.incoming.insert(toState.incoming.begin(), IncomingTrip(node, edge.tripId));
                queue.emplace(edge.to, &toState);
            } else if (newTravelTime < toState.travelTime + options.minTransferTime) {
                // Alternative journey that may result in fewer transfers and therefore faster travel time.
                toState.incoming.emplace_back(node, edge.tripId);
            }
        }
    }

    return state;
}

std::vector<Edge> StopNode::getEdges(Timetable& timetable, const RoutingOptions& options, StopState* state) {
    auto stop = &timetable.stopTimes[stopId];
    auto compare = [](StopTime a, StopTime b) { return a.departureTime < b.departureTime; };

    std::vector<Edge> outgoingEdges;
    std::set<uint64_t> outgoingDirections;

    // Walk to another stop
    for (Edge transfer : transfers) outgoingEdges.push_back(transfer);

    // Alternative journeys
    for (IncomingTrip trip : state->incoming) {
        if (trip.tripId == WALK) continue;

        auto& stopTimes = timetable.trips[trip.tripId].stopTimes;
        uint32_t stopSequence = 0;
        for (int i = 0; i < stopTimes.size(); i++) {
            if (stopTimes[i].stopId == stopId) {
                stopSequence = i + 1;
                break;
            }
        }

        // Skip if final stop
        if (stopSequence >= stopTimes.size()) continue;

        // Get next stop
        StopTime next = stopTimes[stopSequence];

        outgoingEdges.emplace_back(&timetable.stops[next.stopId],
                                   next.arrivalTime - options.startTime - state->travelTime, trip.tripId);
    }

    int32_t timeAtStop = options.startTime + state->travelTime + options.minTransferTime;

    auto iter = std::lower_bound(stop->begin(), stop->end(), StopTime(timeAtStop), compare);

    for (; iter < stop->end() && iter->departureTime < timeAtStop + options.searchTime; iter++) {
        Trip* trip = &timetable.trips[iter->tripId];

        // Max one departure per line and direction
        uint64_t direction = trip->routeId + trip->directionId;
        if (outgoingDirections.contains(direction)) continue;

        // Check date for departure
        if (!timetable.calendarDates[trip->serviceId].contains(options.date)) continue;

        outgoingDirections.insert(direction);

        // Skip if final stop
        if (iter->stopSequence >= trip->stopTimes.size()) continue;

        // Get next stop
        StopTime next = trip->stopTimes[iter->stopSequence];

        // Skip departure if the next stop is the stop that you came from
        if (!state->incoming.empty() && next.stopId == state->incoming[0].from->stopId) continue;

        outgoingEdges.emplace_back(&timetable.stops[next.stopId],
                                   next.arrivalTime - options.startTime - state->travelTime, iter->tripId);
    }
    return outgoingEdges;
}

static StopId stopAreaFromStopPoint(StopId stopId) { return stopId - stopId % 1000 - 1000000000000; }

static bool isStopPoint(StopId stopId) { return stopId % 10000000000000 / 1000000000000 == 2; }

Timetable::Timetable(const std::string& gtfsPath) {
    for (const auto& t : gtfs::Trip::load(gtfsPath)) {
        trips[t.tripId] = {t.serviceId, {}, t.directionId, t.routeId};
    }

    for (const auto& st : gtfs::StopTime::load(gtfsPath)) {
        StopId stopId = stopAreaFromStopPoint(st.stopId);
        StopTime stopTime{st.tripId, st.arrivalTime.timestamp, st.departureTime.timestamp, stopId, st.stopSequence};

        stopTimes[stopId].push_back(stopTime);
        trips[st.tripId].stopTimes.push_back(stopTime);
    }

    for (auto& [stopId, st] : stopTimes) {
        auto compare = [](StopTime a, StopTime b) { return a.departureTime < b.departureTime; };
        std::sort(st.begin(), st.end(), compare);
    }

    for (auto& cd : gtfs::CalendarDate::load(gtfsPath)) {
        calendarDates[cd.serviceId].insert(cd.date.original);
    }

    for (auto& s : gtfs::Stop::load(gtfsPath)) {
        if (isStopPoint(s.stopId)) continue;
        stops[s.stopId] = {s.stopId, s.stopName, s.stopLat, s.stopLon};
    }

    for (auto& t : gtfs::Transfer::load(gtfsPath)) {
        if (t.transferType != 2) continue;

        StopId from = stopAreaFromStopPoint(t.fromStopId);
        StopId to = stopAreaFromStopPoint(t.toStopId);

        if (from == to) continue;

        Edge transfer = {&stops[to], t.minTransferTime, WALK};

        auto& transfers = stops[from].transfers;
        if (!std::any_of(transfers.begin(), transfers.end(), [to](auto t) { return t.to->stopId == to; })) {
            stops[from].transfers.push_back(transfer);
        }
    }

    for (gtfs::Route& r : gtfs::Route::load(gtfsPath)) {
        routes[r.routeId] = r;
    }
}
