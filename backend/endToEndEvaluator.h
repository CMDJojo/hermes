#pragma once
#include <ostream>

#include "people.h"
#include "prox.h"
#include "routing.h"

class E2EE {
   public:
    struct PersonPath {
        StopId firstStop;
        int32_t timeAtFirstStop;
        StopId secondStop;
        int32_t timeAtSecondStop;
        int32_t timeAtEnd;
        std::vector<StopId> extractedPath;

        friend std::ostream& operator<<(std::ostream& os, const PersonPath& path);
        [[nodiscard]] std::string toNiceString(const routing::Timetable& tt) const;
        [[nodiscard]] std::string toNiceString(const routing::Timetable& tt, int32_t timeAtStart) const;
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

        StatConfig statsToCollect; // use bitmasks from E2EE to choose what stats to collect

        routing::RoutingOptions routingOptions;  // Options to use when outing
    };

    static const uint32_t COLLECT_DIST_START_STOPS = 0b1;
    static const uint32_t COLLECT_DIST_END_STOPS = 0b10;
    static const uint32_t COLLECT_SIMPLIFIED_PATHS = 0b100;
    static const uint32_t COLLECT_EXTRACTED_PATHS = 0b1000 | COLLECT_SIMPLIFIED_PATHS;
    static const uint32_t INCLUDE_TT_OPTS_POINTERS = 0b10000;

    static const uint32_t COLLECT_ALL =
        COLLECT_DIST_START_STOPS |
            COLLECT_DIST_END_STOPS |
                COLLECT_SIMPLIFIED_PATHS |
                    COLLECT_EXTRACTED_PATHS |
                        INCLUDE_TT_OPTS_POINTERS;

    const People& people;
    const Prox prox;
    mutable routing::Timetable timetable;
    E2EE(const People& people, routing::Timetable timetable);
    Stats evaluatePerformanceAtPoint(MeterCoord origin, Options opts);
    static void test();
};
