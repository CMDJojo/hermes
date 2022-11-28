#include "routing.h"

#include <algorithm>
#include <functional>
#include <iostream>
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
        if (nodeState->visited && !nodeState->revisit) continue;
        nodeState->visited = true;

        for (Edge& edge : node->getEdges(*this, options, nodeState)) {
            int32_t newTravelTime = nodeState->travelTime + edge.cost;
            StopState& toState = state[edge.to->stopId];

            if (newTravelTime < toState.travelTime) {
                toState.travelTime = newTravelTime;
                toState.incoming.insert(toState.incoming.begin(), IncomingTrip(node, edge.tripId, edge.stopSequence));
                queue.emplace(edge.to, &toState);

            } else if (newTravelTime <= toState.travelTime + edge.to->minTransferTime) {
                // Alternative trips that may result in fewer transfers and therefore faster travel time.

                // Do not add the same trip again when revisiting.
                if (toState.revisit && std::any_of(toState.incoming.begin(), toState.incoming.end(),
                                                   [&edge](IncomingTrip t) { return t.tripId == edge.tripId; }))
                    continue;

                toState.incoming.emplace_back(node, edge.tripId, edge.stopSequence);

                // Revisit the stop if it has already been visited.
                if (toState.visited) {
                    toState.revisit = true;
                    queue.emplace(edge.to, &toState);
                }
            }
        }
    }

    state[start].incoming.clear();
    return state;
}

std::vector<Edge> StopNode::getEdges(Timetable& timetable, const RoutingOptions& options, StopState* state) {
    auto stop = &timetable.stopTimes[stopId];

    std::vector<Edge> outgoingEdges;
    std::set<uint64_t> outgoingDirections;

    // Walk to another stop
    for (Edge transfer : transfersType2) outgoingEdges.push_back(transfer);

    // Alternative trips
    for (IncomingTrip trip : state->incoming) {
        if (trip.tripId == WALK) continue;

        // Transfers to trips that is waiting for this trip to arrive
        handleTransferType1(timetable, options, state, outgoingEdges, trip);

        auto& stopTimes = timetable.trips[trip.tripId].stopTimes;

        // Skip if final stop
        if (trip.stopSequence >= stopTimes.size()) continue;

        // Get next stop
        StopTime next = stopTimes[trip.stopSequence];

        outgoingEdges.emplace_back(&timetable.stops[next.stopId],
                                   next.arrivalTime - options.startTime - state->travelTime, trip.tripId,
                                   next.stopSequence);
    }

    int32_t timeAtStop = options.startTime + state->travelTime + minTransferTime;

    auto compare = [](StopTime a, StopTime b) { return a.departureTime < b.departureTime; };
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
                                   next.arrivalTime - options.startTime - state->travelTime, iter->tripId,
                                   next.stopSequence);
    }
    return outgoingEdges;
}

inline void StopNode::handleTransferType1(Timetable& timetable, const RoutingOptions& options, const StopState* state,
                                          std::vector<Edge>& outgoingEdges, const IncomingTrip& trip) {
    auto transfers = transfersType1.find(trip.tripId);
    if (transfers != transfersType1.end()) {
        for (TripId toTripId : transfers->second) {
            auto& stopTimes = timetable.trips[toTripId].stopTimes;
            int32_t stopSequence = 0;
            for (int i = 0; i < stopTimes.size(); i++) {
                if (stopTimes[i].stopId == stopId &&
                    stopTimes[i].departureTime >= options.startTime + state->travelTime) {
                    stopSequence = i + 1;
                    break;
                }
            }

            StopTime next = stopTimes[stopSequence];

            outgoingEdges.emplace_back(&timetable.stops[next.stopId],
                                       next.arrivalTime - options.startTime - state->travelTime, toTripId,
                                       next.stopSequence);
        }
    }
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
        StopId from = stopAreaFromStopPoint(t.fromStopId);
        StopId to = stopAreaFromStopPoint(t.toStopId);

        if (t.transferType == 1) {
            if (from != to) continue;
            stops[from].transfersType1[t.fromTripId].push_back(t.toTripId);

        } else if (t.transferType == 2) {
            // Set change margin for stop area
            if (t.fromStopId == t.toStopId && !isStopPoint(t.fromStopId) && t.minTransferTime) {
                stops[t.fromStopId].minTransferTime = t.minTransferTime;
                continue;
            }

            if (from == to) continue;

            Edge transfer = {&stops[to], t.minTransferTime, WALK, 0};

            auto& transfers = stops[from].transfersType2;
            if (!std::any_of(transfers.begin(), transfers.end(), [to](auto t) { return t.to->stopId == to; })) {
                stops[from].transfersType2.push_back(transfer);
            }
        }
    }

    for (gtfs::Route& r : gtfs::Route::load(gtfsPath)) {
        routes[r.routeId] = r;
    }
}

std::string routing::prettyTravelTime(int32_t time) {
    if (time < 60) {
        return "< 1 min";
    }
    auto min = time / 60;
    if (min < 60) {
        return std::to_string(min) + " min";
    } else {
        return std::to_string(min / 60) + " h " + std::to_string(min % 60) + " min";
    }
}
