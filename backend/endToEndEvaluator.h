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

        friend std::ostream& operator<<(std::ostream& os, const PersonPath& path);
        std::string toNiceString(const routing::Timetable& tt) const;
    };

    struct Stats {
        uint64_t personsWithinRange;
        uint64_t excludedWithinMinimumRange;
        uint64_t personsCanGoWithBus;
        uint64_t uniqueSpots;

        std::unordered_map<uint64_t, int> distNumberOfStartStops;
        std::unordered_map<uint64_t, int> distNumberOfEndStops;
        uint64_t hasThisAsOptimal;

        std::vector<PersonPath> allPaths;

        Stats() = default;
        friend std::ostream& operator<<(std::ostream& os, const Stats& stats);
    };

    struct Options {
        double moveSpeed;  // meter per seconds, very important!!!
        int moveableDistance;
        int minimumRange;

        routing::RoutingOptions routingOptions;  // Options to use when outing
    };

    const People& people;
    const Prox prox;
    mutable routing::Timetable timetable;
    E2EE(const People& people, routing::Timetable timetable);
    Stats evaluatePerformanceAtPoint(MeterCoord origin, StopId interestingStop, Options opts);
    static void test();
};
