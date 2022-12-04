#pragma once
#include <ostream>
#include <map>

#include "people.h"
#include "prox.h"
#include "routing.h"

using SegmentId = uint64_t;

class E2EE {
   public:
    struct ShapeSegment;

    struct PersonPath {
        StopId firstStop;
        int32_t timeToFirstStop;  // time taken, home->first stop
        StopId secondStop;
        int32_t timeToSecondStop;  // time taken, first->second stop
        int32_t timeToGoal;        // time taken, second->goal
        int32_t timeAtGoal;        // total time taken, home->goal
        int32_t timestampAtGoal;   // timestamp at goal
        std::vector<StopId> extractedPath;

        friend std::ostream& operator<<(std::ostream& os, const PersonPath& path);
        [[nodiscard]] std::string toNiceString(const routing::Timetable& tt) const;
    };

    struct ShapeSegment {
        StopId startStop{};
        StopId endStop{};
        TripId tripId{};
        //double startDistTravelled{};
        //double endDistTravelled{};
        int32_t startIdx{};
        int32_t endIdx{};
        int32_t passengerCount = 1;
        
        ShapeSegment() = default;
        ShapeSegment(StopId start_stop, StopId end_stop, TripId trip_id, int32_t start_idx, int32_t end_idx)
            : startStop(start_stop), endStop(end_stop), tripId(trip_id), startIdx(start_idx), endIdx(end_idx) {}
    };

    struct Options;

    struct Stats {
        uint64_t personsWithinRange = 0;
        uint64_t excludedWithinMinimumRange = 0;
        uint64_t personsCanGoWithBus = 0;
        uint64_t uniqueSpots = 0;

        std::map<uint64_t, int, std::less<>> distNumberOfStartStops;
        std::map<uint64_t, int, std::less<>> distNumberOfEndStops;
        uint64_t hasThisAsOptimal = 0;

        std::vector<PersonPath> allPaths;

        routing::Timetable* tt = nullptr;
        E2EE::Options* opts = nullptr;

        std::unordered_map<SegmentId, ShapeSegment> shapeSegments;

        Stats() = default;
        friend std::ostream& operator<<(std::ostream& os, const Stats& stats);
        std::string prettyString();
    };

    using StatConfig = uint32_t;

    struct Options {
        StopId interestingStop;
        double moveSpeed;  // meter per seconds, very important!!!
        int searchRange;
        int moveableDistance;
        int minimumRange;

        StatConfig statsToCollect;  // use bitmasks from E2EE to choose what stats to collect

        routing::RoutingOptions routingOptions;  // Options to use when outing
    };

    static const uint32_t COLLECT_DIST_START_STOPS = 0b1;
    static const uint32_t COLLECT_DIST_END_STOPS = 0b10;
    static const uint32_t COLLECT_SIMPLIFIED_PATHS = 0b100;
    static const uint32_t COLLECT_EXTRACTED_PATHS = 0b1000;
    static const uint32_t COLLECT_EXTRACTED_SHAPES = 0b10000;
    static const uint32_t INCLUDE_TT_OPTS_POINTERS = 0b100000;

    static const uint32_t COLLECT_ALL = COLLECT_DIST_START_STOPS | COLLECT_DIST_END_STOPS | COLLECT_SIMPLIFIED_PATHS |
                                        COLLECT_EXTRACTED_PATHS | COLLECT_EXTRACTED_SHAPES | INCLUDE_TT_OPTS_POINTERS;

    const People& people;
    const Prox prox;
    mutable routing::Timetable timetable;
    E2EE(const People& people, routing::Timetable timetable);
    Stats evaluatePerformanceAtPoint(MeterCoord origin, Options opts);
    static void test();
};
