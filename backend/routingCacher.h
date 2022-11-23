#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "gtfsTypes.h"
#include "routing.h"

namespace routingCacher {

using namespace routing;

/*
 * {
 *      stopId [string]: {
 *          time [i32],
 *          incoming: {
 *              from: stopId [u64],
 *              trip: tripId [u64],
 *              tripStr: string
 *          }
 *      }
 * }
 */

struct ParsedIncomingTrip {
    StopId from;
    TripId tripId;
    bool operator==(const ParsedIncomingTrip& rhs) const;
    bool operator!=(const ParsedIncomingTrip& rhs) const;
};

struct ParsedStopState {
    int32_t travelTime;
    std::vector<ParsedIncomingTrip> incoming;
    bool operator==(const ParsedStopState& rhs) const;
    bool operator!=(const ParsedStopState& rhs) const;
};

std::string toJson(std::unordered_map<StopId, StopState> map);
std::unordered_map<StopId, ParsedStopState> fromJson(const std::string& json);
std::unordered_map<StopId, ParsedStopState> toPSS(std::unordered_map<StopId, StopState> map);

void test();
}  // namespace RoutingCacher

