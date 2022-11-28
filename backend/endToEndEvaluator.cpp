#include "endToEndEvaluator.h"

#include <utility>

#include "routing.h"

E2EE::E2EE(const People& people, routing::Timetable timetable) : people(people), timetable(std::move(timetable)) {}

E2EE::Stats E2EE::evaluatePerformanceAtPoint(MeterCoord origin, StopId interestingStop, E2EE::Options opts) {
    Stats ret{};

    // find all persons within range
    std::vector<Person> allPersons = people.personsInCircle(origin, opts.moveableDistance);
    ret.personsWithinRange = allPersons.size();
    std::cout << ret.personsWithinRange << " persons within range" << std::endl;

    // only keep persons further from a job than a certain range
    std::vector<Person> filteredPersons;
    uint64_t excluded = 0;
    for (auto person : allPersons) {
        if (!person.work_coord.distanceToLEQ(person.home_coord, opts.minimumRange)) {
            filteredPersons.push_back(person);
        } else {
            excluded++;
        }
    }
    ret.excludedWithinMinimumRange = excluded;

    // find all coords where ppl live within range
    std::vector<MeterCoord> populatedCoords = people.populatedCoordsInCircle(origin, opts.moveableDistance);

    // for each coord someone lives in, calculate what stops they might go to and time taken to do so
    std::unordered_map<MeterCoord, std::vector<std::pair<StopId, int32_t>>> walkableStops;
    for (auto coord : populatedCoords) walkableStops.emplace(coord, closeEnoughStuff(coord, opts));

    struct TemporaryPath {
        int32_t currentTime;
        StopId firstStop;
        int32_t timeAtFirstStop;
        StopId secondStop;
        int32_t timeAtSecondStop;
        int32_t timeAtEnd;
        std::unordered_map<StopId, int32_t> secondTimes;

        PersonPath toPersonPath() {
            return PersonPath{firstStop, timeAtFirstStop, secondStop, timeAtSecondStop, timeAtEnd};
        }
    };

    std::unordered_map<StopId, std::unordered_map<StopId, routing::StopState>> dijkstraCache;

    routing::RoutingOptions routingOptions = {opts.startTime, 20221118, 30 * 60, 5 * 60};
    uint64_t unreachableTargets = 0;
    int n = 0;
    for (auto person : filteredPersons) {
        if (++n % 100 == 0) std::cout << "Testing person " << n << std::endl;

        // all possible targets
        std::vector<std::pair<StopId, int32_t>> possibleVTGoals = closeEnoughStuff(person.work_coord, opts);
        if (possibleVTGoals.empty()) {
            unreachableTargets++;
            continue;
        }

        std::vector<TemporaryPath> firstStops;

        // first stop candidates
        for (auto [fscID, fscTime] : walkableStops[person.home_coord]) {
            // routingOptions.startTime = opts.startTime + fscTime;
            // auto dijRes = timetable.dijkstra(fscID, routingOptions);
            if (!dijkstraCache.contains(fscID)) {
                dijkstraCache.insert_or_assign(fscID, timetable.dijkstra(fscID, routingOptions));
            }

            std::unordered_map<StopId, routing::StopState> dijRes = dijkstraCache[fscID];

            std::unordered_map<StopId, int32_t> secondTimes;

            for (auto [escID, escTime] : possibleVTGoals) {
                if (dijRes.contains(escID)) {
                    secondTimes.emplace(escID, dijRes[escID].travelTime);
                }
            }

            // 5 days lol
            TemporaryPath path{fscTime, fscID, fscTime, 0, 0, 0, secondTimes};

            firstStops.push_back(path);
        }

        TemporaryPath fastest = {60 * 60 * 24 * 5, 0, 0, {}};

        // map second stop to whichever temporary path comes there faster
        std::unordered_map<StopId, TemporaryPath> fastestHere;
        for (auto [sscID, sscTime] : possibleVTGoals) {
            for (const auto& tp : firstStops) {
                // if second is reachable from first
                if (tp.secondTimes.contains(sscID)) {
                    TemporaryPath newPath = {tp.currentTime + tp.secondTimes.at(sscID) + sscTime,
                                             tp.firstStop,
                                             tp.timeAtFirstStop,
                                             sscID,
                                             tp.currentTime + tp.secondTimes.at(sscID),
                                             tp.currentTime + tp.secondTimes.at(sscID) + sscTime,
                                             {}};
                    if (newPath.currentTime < fastest.currentTime) {
                        fastest = newPath;
                    }
                }
            }
        }

        // FIXME: dont ignore zeroes (skipped earlier on)
        {
            auto key = walkableStops[person.home_coord].size();
            auto search = ret.distNumberOfStartStops.find(key);
            int next = (search == ret.distNumberOfStartStops.end()) ? 1 : (search->second + 1);
            ret.distNumberOfStartStops.insert_or_assign(key, next);
        }

        {
            auto key = possibleVTGoals.size();
            auto search = ret.distNumberOfEndStops.find(key);
            int next = (search == ret.distNumberOfEndStops.end()) ? 1 : (search->second + 1);
            ret.distNumberOfEndStops.insert_or_assign(key, next);
        }

        PersonPath personPath = fastest.toPersonPath();
        ret.allPaths.push_back(personPath);

        if (personPath.firstStop == interestingStop) {
            ret.hasThisAsOptimal++;
        }
        ret.personsCanGoWithBus++;
    }

    return ret;
}

std::vector<std::pair<StopId, int32_t>> E2EE::closeEnoughStuff(const MeterCoord& pt, const Options& opts) {
    std::vector<std::pair<StopId, int32_t>> timeToWalkToStops;
    for (auto [id, node] : timetable.stops) {
        DMSCoord c(node.lat, node.lon);
        if (pt.distanceToLEQ(c.toMeter(), opts.moveableDistance)) {
            double dist = c.toMeter().distanceTo(pt);
            double time = dist / opts.moveSpeed;
            timeToWalkToStops.emplace_back(node.stopId, time);
        }
    }
    return timeToWalkToStops;
}

std::ostream& operator<<(std::ostream& os, const E2EE::PersonPath& path) {
    os << "Path {firstStop:" << path.firstStop << ",timeAtFirstStop:" << path.timeAtFirstStop
       << ",secondStop:" << path.secondStop << ",timeAtSecondStop:" << path.timeAtSecondStop
       << ",timeAtEnd:" << path.timeAtEnd << " }";
    return os;
}

std::string E2EE::PersonPath::toNiceString(const routing::Timetable& tt) const {
    if (this->firstStop == 0) return "Invalid path, firstStop doesn't exist";
    if (this->secondStop == 0) return "Invalid path, secondStop doesn't exist";

    return "Walk " + std::to_string(this->timeAtFirstStop) + " s, take bus from " + tt.stops.at(this->firstStop).name +
           " to arrive at " + tt.stops.at(this->secondStop).name + " after " +
           routing::prettyTravelTime(this->timeAtSecondStop) + ". You'll be at work after " +
           routing::prettyTravelTime(this->timeAtEnd) + ".";
}

void E2EE::test() {
    std::cout << "[TEST] Testing endToEndEvaluator at a random point" << std::endl;
    std::cout << "[TEST] Loading TimeTable and People..." << std::endl;

    routing::Timetable timetable("data/raw");
    People people("data/raw/Ast_bost.txt");
    std::cout << "[TEST] E2EE..." << std::endl;

    // dr forselius gata
    StopId target = 9021014002080000;
    double latitude = 57.678623;
    double longitude = 11.983399;
    auto originCoord = DMSCoord(latitude, longitude).toMeter();

    E2EE::Options opts = {1, 100, 0, 60 * 60 * 10};

    E2EE obj(people, timetable);

    std::cout << "[TEST] Calculating..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    Stats s = obj.evaluatePerformanceAtPoint(originCoord, target, opts);
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "[TEST] Done, took " << duration.count() << " μs" << std::endl;

    std::cout << "[TEST] " << s << std::endl;

    std::cout << "[TEST] Some sample paths:" << std::endl;
    int count = 10;
    for (auto path : s.allPaths) {
        std::cout << path.toNiceString(timetable) << std::endl;
        if (count-- < 0) break;
    }

    std::cout << "[TEST] Ended" << std::endl;
}

std::ostream& operator<<(std::ostream& os, const E2EE::Stats& stats) {
    os << "Stats {" << stats.personsWithinRange << " persons within range, " << stats.excludedWithinMinimumRange
       << " excluded due to work being within minimum range, " << stats.personsCanGoWithBus << " could go with bus, "
       << stats.hasThisAsOptimal << " had this as optimal stop within range, n:o start stop dist: {";

    for (auto [num, ppl] : stats.distNumberOfStartStops) {
        os << num << ":" << ppl << ",";
    }
    os << "}, n:o end stop dist: {";

    for (auto [num, ppl] : stats.distNumberOfEndStops) {
        os << num << ":" << ppl << ",";
    }
    os << "}, " << stats.allPaths.size() << " paths}";
    return os;
}
