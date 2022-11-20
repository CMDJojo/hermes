#include "routing.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>
#include <set>
#include <vector>

#include "gtfsTypes.h"

using namespace routing;

void Timetable::dijkstra(StopNode* start, const RoutingOptions& options) {
    auto compare = [](StopNode* a, StopNode* b) { return a->travelTime > b->travelTime; };
    std::priority_queue<StopNode*, std::vector<StopNode*>, decltype(compare)> queue;

    for (auto& node : stops) {
        node.second.travelTime = std::numeric_limits<int32_t>::max();
        node.second.visited = false;
    }

    start->travelTime = 0;
    queue.push(start);

    while (!queue.empty()) {
        StopNode* node = queue.top();
        queue.pop();

        // Ignore duplicates
        if (node->visited) continue;
        node->visited = true;

        for (Edge& edge : node->getEdges(*this, options)) {
            int32_t newTravelTime = node->travelTime + edge.cost;
            if (newTravelTime < edge.to->travelTime) {
                edge.to->travelTime = newTravelTime;
                edge.to->incoming.insert(edge.to->incoming.begin(), IncomingTrip(node, edge.tripId));
                queue.push(edge.to);
            } else if (newTravelTime < edge.to->travelTime + edge.to->minTransferTime) {
                // Alternative journey that may result in fewer transfers and therefore faster travel time.
                edge.to->incoming.emplace_back(node, edge.tripId);
            }
        }
    }
}

std::vector<Edge> StopNode::getEdges(Timetable& graph, const RoutingOptions& options) {
    auto stop = &graph.stopTimes[stopId];
    auto compare = [](StopTime a, StopTime b) { return a.departureTime < b.departureTime; };

    std::vector<Edge> outgoingEdges;
    std::set<uint64_t> outgoingDirections;

    // Alternative journeys
    for (IncomingTrip trip : incoming) {
        auto& stopTimes = graph.trips[trip.tripId].stopTimes;
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

        outgoingEdges.emplace_back(&graph.stops[next.stopId], next.arrivalTime - options.startTime - travelTime,
                                   trip.tripId);
    }

    auto iter = std::lower_bound(stop->begin(), stop->end(), StopTime(options.startTime + travelTime + minTransferTime),
                                 compare);

    for (; iter < stop->end() && iter->departureTime < options.startTime + travelTime + options.searchTime; iter++) {
        Trip* trip = &graph.trips[iter->tripId];

        // Max one departure per line and direction
        uint64_t direction = trip->routeId + trip->directionId;
        if (outgoingDirections.contains(direction)) continue;
        outgoingDirections.insert(direction);

        // Check date for departure
        if (graph.calendarDates[trip->serviceId] == options.date) continue;

        // Skip if final stop
        if (iter->stopSequence >= trip->stopTimes.size()) continue;

        // Get next stop
        StopTime next = trip->stopTimes[iter->stopSequence];

        // Skip departure if the next stop is the stop that you came from
        if (!incoming.empty() && next.stopId == incoming[0].from->stopId) continue;

        outgoingEdges.emplace_back(&graph.stops[next.stopId], next.arrivalTime - options.startTime - travelTime,
                                   iter->tripId);
    }
    return outgoingEdges;
}

Timetable::Timetable(const std::string& gtfsPath) {
    for (const auto& t : gtfs::Trip::load(gtfsPath)) {
        trips[t.tripId] = {t.serviceId, {}, t.directionId, t.routeId};
    }

    for (const auto& st : gtfs::StopTime::load(gtfsPath)) {
        StopId stopId = st.stopId - st.stopId % 1000;
        StopTime stopTime{st.tripId, st.arrivalTime.timestamp, st.departureTime.timestamp, stopId, st.stopSequence};

        stopTimes[stopId].push_back(stopTime);
        trips[st.tripId].stopTimes.push_back(stopTime);
    }

    for (auto& [stopId, st] : stopTimes) {
        auto compare = [](StopTime a, StopTime b) { return a.departureTime < b.departureTime; };
        std::sort(st.begin(), st.end(), compare);
    }

    for (auto& cd : gtfs::CalendarDate::load(gtfsPath)) {
        calendarDates[cd.serviceId] = cd.date;
    }

    for (auto& s : gtfs::Stop::load(gtfsPath)) {
        StopId stopId = s.stopId - s.stopId % 1000;
        stops[stopId] = {stopId, 0, {}, 5 * 60, false};
    }
}
