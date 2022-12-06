#include "endToEndEvaluator.h"

#include <sstream>
#include <utility>

#include "routing.h"

using namespace routing;

static void extractShape(Timetable& tt, StopId to, std::unordered_map<StopId, StopState>& graph,
                         std::unordered_map<SegmentId, E2EE::ShapeSegment>& segments) {
    if (graph.find(to) == graph.end()) return;

    StopState* current = &graph.at(to);
    if (current->incoming.empty()) return;

    TripId currentTrip = current->incoming.front().tripId;
    StopId currentId = to;

    while (!current->incoming.empty()) {
        // we came from "from" and are going to "current"
        IncomingTrip from = current->incoming[0];
        for (const IncomingTrip& node : current->incoming) {
            if (node.tripId == currentTrip && currentTrip != WALK) {
                from = node;
                break;
            }
        }

        if (from.tripId != WALK) {
            Trip& trip = tt.trips[from.tripId];
            SegmentId segmentId = trip.routeId + from.stopSequence * 10 + trip.directionId;

            auto iter = segments.find(segmentId);
            if (iter != segments.end()) {
                iter->second.passengerCount++;
            } else {
                int32_t startIdx, endIdx;

                StopTime& start = trip.stopTimes[from.stopSequence - 2];
                StopTime& end = trip.stopTimes[from.stopSequence - 1];

                auto& shape = tt.shapes.at(trip.shapeId);

                if (end.shapeDistTravelled == 0 && start.shapeDistTravelled == 0) {
                    auto sqDist = [](DMSCoord& a, DMSCoord& b) {
                        double deltaLat = a.latitude - b.latitude;
                        double deltaLon = a.longitude - b.longitude;
                        return deltaLat * deltaLat + deltaLon * deltaLon;
                    };

                    auto findClosestPoint = [&sqDist](DMSCoord& point) {
                        return [&point, &sqDist](std::pair<double, DMSCoord> a, std::pair<double, DMSCoord> b) {
                            return sqDist(a.second, point) < sqDist(b.second, point);
                        };
                    };

                    auto& startCoord = tt.stopPoints.at(start.stopPoint);
                    auto startPoint = std::min_element(shape.begin(), shape.end(), findClosestPoint(startCoord));
                    startIdx = (int32_t)std::distance(shape.begin(), startPoint);

                    auto& endCoord = tt.stopPoints.at(end.stopPoint);
                    auto endPoint = std::min_element(shape.begin() + startIdx, shape.end(), findClosestPoint(endCoord));
                    endIdx = (int32_t)std::distance(shape.begin(), endPoint) + 1;
                } else {
                    auto compare = [](std::pair<double, DMSCoord> a, std::pair<double, DMSCoord> b) {
                        return a.first < b.first;
                    };

                    auto startBound = std::lower_bound(
                        shape.begin(), shape.end(), std::make_pair(start.shapeDistTravelled, DMSCoord(0, 0)), compare);
                    startIdx = (int32_t)std::distance(shape.begin(), startBound);

                    auto endBound = std::upper_bound(shape.begin(), shape.end(),
                                                     std::make_pair(end.shapeDistTravelled, DMSCoord(0, 0)), compare);
                    endIdx = (int32_t)std::distance(shape.begin(), endBound);
                }

                segments[segmentId] = {from.from->stopId, currentId, from.tripId, startIdx, endIdx, from.stopSequence};
            }
        } else {
            SegmentId segmentId = from.from->stopId ^ (currentId << 32);

            auto iter = segments.find(segmentId);
            if (iter != segments.end()) {
                iter->second.passengerCount++;
            } else {
                segments[segmentId] = {from.from->stopId, currentId, WALK, 0, 0, 0};
            }
        }

        currentTrip = from.tripId;
        currentId = from.from->stopId;
        current = &graph[currentId];
    }
}

std::vector<StopId> extractPath(Timetable& timetable, StopId stopId, std::unordered_map<StopId, StopState>& graph) {
    if (graph.find(stopId) == graph.end()) return {};
    
    StopState* current = &graph.at(stopId);
    if (current->incoming.empty()) return {};
    
    TripId currentTrip = current->incoming.front().tripId;
    std::vector<StopId> legs;
    legs.push_back(stopId);

    while (!current->incoming.empty()) {
        for (const IncomingTrip& node : current->incoming) {
            if (node.tripId == currentTrip && currentTrip != WALK) {
                legs.push_back(node.from->stopId);
                break;
            }
        }
        StopId fromStopId = current->incoming[0].from->stopId;
        legs.push_back(fromStopId);
        current = &graph[fromStopId];
    }

    std::reverse(legs.begin(), legs.end());
    return legs;
}

E2EE::E2EE(const People& people, Timetable timetable)
    : people(people), timetable(std::move(timetable)), prox("data/raw") {}

E2EE::Stats E2EE::evaluatePerformanceAtPoint(MeterCoord origin, E2EE::Options opts) {
    // The Stats struct to return, fields will be updated throughout
    Stats ret{};

    // If pointers should be included in Stats, add them
    if (opts.statsToCollect & INCLUDE_TT_OPTS_POINTERS) {
        ret.opts = &opts;
        ret.tt = &timetable;
    }

    // find all persons within the specified search range
    std::vector<Person> allPersons = people.personsInCircle(origin, opts.searchRange);
    ret.personsWithinRange = allPersons.size();

    // now we filter so we only keep persons whose work is further from home than the specified minimum range
    std::vector<Person> filteredPersons;
    for (auto person : allPersons) {
        if (!person.work_coord.distanceToLEQ(person.home_coord, opts.minimumRange)) filteredPersons.push_back(person);
    }
    // excluded persons = persons before filtering - persons after filtering
    ret.excludedWithinMinimumRange = ret.personsWithinRange - filteredPersons.size();

    // find all coords where ppl live on within range
    std::vector<MeterCoord> populatedCoords = people.populatedCoordsInCircle(origin, opts.moveableDistance);
    ret.uniqueSpots = populatedCoords.size();

    // for each coord someone lives on, calculate what stops they might go to and time taken to do so
    std::unordered_map<MeterCoord, std::vector<std::pair<StopId, double>>> walkableStops;
    walkableStops.reserve(populatedCoords.size());

    for (auto coord : populatedCoords)
        walkableStops.emplace(
            coord,
            prox.stopsIDAndDistanceMultipliedWithAFactorWhichInFactIsJustTheWalkSpeedWithinACertainRangeInclusiveButRounded(
                coord, opts.moveableDistance, opts.moveSpeed));

    std::unordered_map<StopId, std::unordered_map<StopId, StopState>> dijkstraCache;

    RoutingOptions& routingOptions = opts.routingOptions;
    uint64_t unreachableTargets = 0;
    int n = 0;
    int i = 0;
    int nptg = 0;
    for (auto person : filteredPersons) {
        if (++n % 1000 == 0) std::cout << "Testing person " << n << std::endl;

        // all possible targets
        std::vector<std::pair<StopId, double>> possibleVTGoals =
            prox.stopsIDAndDistanceMultipliedWithAFactorWhichInFactIsJustTheWalkSpeedWithinACertainRangeInclusiveButRounded(
                person.work_coord, opts.moveableDistance, opts.moveSpeed);

        if (possibleVTGoals.empty()) {
            unreachableTargets++;
            continue;
        }

        // Keep track of the fastest path found yet
        PersonPath fastest{0, 0, 0, 0, 0, 60 * 60 * 24 * 5, 0};

        // Loop over all possible first stops
        for (auto [firstStopId, firstStopTime] : walkableStops[person.home_coord]) {
            // If no dijkstra exists for that stop, cache it
            if (!dijkstraCache.contains(firstStopId)) {
                if (++i % 10 == 0) std::cout << "Dijkstra " << i << std::endl;
                dijkstraCache.insert_or_assign(firstStopId, timetable.dijkstra(firstStopId, routingOptions));
            }

            // Get a hold on the dijkstra result for that first stop
            std::unordered_map<StopId, StopState>& dijRes = dijkstraCache[firstStopId];

            // For each possible end stop...
            for (auto [endStopId, timeToGoal] : possibleVTGoals) {
                // if second is reachable from firsts
                if (dijRes.contains(endStopId)) {
                    int32_t timeToEndStop = dijRes.at(endStopId).travelTime;

                    auto firstStopTimeInt = static_cast<int32_t>(firstStopTime);
                    auto timeToGoalInt = static_cast<int32_t>(timeToGoal);

                    int32_t timeAtGoal = firstStopTimeInt + timeToEndStop + timeToGoalInt;

                    if (timeAtGoal < fastest.timeAtGoal) {
                        fastest = {firstStopId,   firstStopTimeInt, endStopId, timeToEndStop,
                                   timeToGoalInt,
                                   timeAtGoal,
                                   timeAtGoal + opts.routingOptions.startTime,
                                   {}};
                    }
                }
            }
        }

        if (fastest.firstStop == 0) {
            //TODO: Add this as a stat
            std::cout << "No path to goal (" << (++nptg) << ")\n";
        }

        if (opts.statsToCollect & COLLECT_DIST_START_STOPS) {
            auto key = walkableStops[person.home_coord].size();
            auto search = ret.distNumberOfStartStops.find(key);
            int next = (search == ret.distNumberOfStartStops.end()) ? 1 : (search->second + 1);
            ret.distNumberOfStartStops.insert_or_assign(key, next);
        }

        if (opts.statsToCollect & COLLECT_DIST_END_STOPS) {
            auto key = possibleVTGoals.size();
            auto search = ret.distNumberOfEndStops.find(key);
            int next = (search == ret.distNumberOfEndStops.end()) ? 1 : (search->second + 1);
            ret.distNumberOfEndStops.insert_or_assign(key, next);
        }

        if (opts.statsToCollect & COLLECT_EXTRACTED_PATHS) {
            if (fastest.firstStop != 0 && fastest.firstStop != fastest.secondStop) { //so extract doesn't crash
                fastest.extractedPath = extractPath(timetable, fastest.secondStop, dijkstraCache.at(fastest.firstStop));
            }
        }

        if (opts.statsToCollect & COLLECT_EXTRACTED_SHAPES) {
            if (fastest.firstStop != 0 && fastest.firstStop != fastest.secondStop) {
                extractShape(timetable, fastest.secondStop, dijkstraCache.at(fastest.firstStop), ret.shapeSegments);
            }
        }

        if (opts.statsToCollect & COLLECT_SIMPLIFIED_PATHS && fastest.firstStop) {
            ret.allPaths.push_back(fastest);
        }

        if (fastest.firstStop == opts.interestingStop) {
            ret.hasThisAsOptimal++;
        }
        ret.personsCanGoWithBus++;
    }
    return ret;
}

std::ostream& operator<<(std::ostream& os, const E2EE::PersonPath& path) {
    os << "firstStop: " << path.firstStop << " timeToFirstStop: " << path.timeToFirstStop
       << " secondStop: " << path.secondStop << " timeToSecondStop: " << path.timeToSecondStop
       << " timeToGoal: " << path.timeToGoal << " timeAtGoal: " << path.timeAtGoal
       << " timestampAtGoal: " << path.timestampAtGoal << " extractedPath size: " << path.extractedPath.size();
    return os;
}

#define TIME(n) \
    std::to_string((n) / (60 * 60)) + ":" + ((((n) / 60) % 60) < 10 ? "0" : "") + (std::to_string(((n) / 60) % 60))

std::string E2EE::PersonPath::toNiceString(const Timetable& tt) const {
    if (this->firstStop == 0) {
        return "Invalid path, firstStop doesn't exist";
    }
    if (this->secondStop == 0) {
        return "Invalid path, secondStop doesn't exist";
    }

    std::stringstream s;
    s << "Walk " << this->timeToFirstStop << " s and take transport from " << tt.stops.at(this->firstStop).name;
    s << " to arrive at " << tt.stops.at(this->secondStop).name << " after ";
    s << prettyTravelTime(this->timeToSecondStop) << ". Walk for " << this->timeToGoal << "s to arrive ";
    s << "at the goal at " << TIME(timestampAtGoal) << ".";

    if (!this->extractedPath.empty()) {
        s << " Path taken: Home";
        for (auto stopID : this->extractedPath) {
            if (stopID == WALK) {
                s << "->WALK";
            } else {
                s << "->" << tt.stops.at(stopID).name;
            }
        }
    }
    return s.str();
}

void E2EE::test() {
    std::cout << "[TEST] Testing endToEndEvaluator at a random point" << std::endl;
    std::cout << "[TEST] Loading TimeTable and People..." << std::endl;

    Timetable timetable("data/raw");
    People people("data/raw/Ast_bost.txt");
    std::cout << "[TEST] E2EE..." << std::endl;

    // dr forselius gata
    /*
    StopId target = 9021014002080000;
    double latitude = 57.678623;
    double longitude = 11.983399;
     */

    // mossen Mossen,57.681522,11.984383,9021014004830000
    StopId target = 9021014004830000;
    double latitude = 57.681522;
    double longitude = 11.984383;
    auto originCoord = DMSCoord(latitude, longitude).toMeter();

    RoutingOptions ropts = {60 * 60 * 10, 20221118, 30 * 60, 5 * 60};
    E2EE::Options opts = {target, 0.6, 600, 600, 0,
                          COLLECT_ALL & (~COLLECT_EXTRACTED_SHAPES), ropts};

    E2EE obj(people, timetable);

    std::cout << "[TEST] Calculating..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    Stats s = obj.evaluatePerformanceAtPoint(originCoord, opts);
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "[TEST] Done, took " << duration.count() << " μs" << std::endl;

    std::cout << "[TEST] Testing pretty printer..." << std::endl;
    std::cout << s.prettyString() << std::endl;

    std::cout << "[TEST] Ended" << std::endl;
}

std::ostream& operator<<(std::ostream& os, const E2EE::Stats& stats) {
    os << "Stats {" << stats.personsWithinRange << " persons within range, living in " << stats.uniqueSpots
       << "unique spots, " << stats.excludedWithinMinimumRange << " excluded due to work being within minimum range, "
       << stats.personsCanGoWithBus << " could go with bus, " << stats.hasThisAsOptimal
       << " had this as optimal stop within range, n:o start stop dist: {";

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

// print with fraction
#define PWF(n, w) n << " (" << static_cast<uint64_t>(static_cast<double>(n) * 100 / static_cast<double>(w)) << "%)"

std::string E2EE::Stats::prettyString() {
    std::stringstream s;
    s << "STATS FOR EVALUATION OF VÄSTTRAFIK PERFORMANCE:\n";
    std::string is;
    StopId isid = 0;
    if (opts != nullptr && (isid = opts->interestingStop) != 0) {
        is = tt != nullptr && tt->stops.contains(isid) ? (tt->stops.at(isid).name) : "[ID:" + std::to_string(isid) + "]";
        s << "Interesting stop: " + is + "\n";
        s << "Search range: " << opts->searchRange << "m\n";
    } else {
        s << "Options not present, interesting stop unknown, search range unknown\n";
    }

    s << std::to_string(personsWithinRange) + " persons within range, of which\n";
    s << " * " + std::to_string(uniqueSpots) + " unique living coordinates were found\n";
    s << " * " << PWF(excludedWithinMinimumRange, personsWithinRange)
      << " persons were discarded due to working too close to home";
    if (opts != nullptr) s << " (within " << (opts->minimumRange) << " m)";
    uint64_t pplTested = personsWithinRange - excludedWithinMinimumRange;
    s << "\n";
    s << " * " << PWF(personsCanGoWithBus, pplTested) << " could get to destination by bus";
    if (opts != nullptr) s << " by walking at most " << opts->moveableDistance << "m";
    s << "\n";

    if (isid != 0)
        s << " * " << PWF(hasThisAsOptimal, personsCanGoWithBus) << " had " << is << " as the optimal first stop\n";
    s << "\n";
    s << allPaths.size() << " paths stored";
    if (!allPaths.empty()) {
        s << ", here is a sample of them:";
        if (tt == nullptr) {
            for (int i = 0; i < 10 && i < allPaths.size(); i++) {
                s << "\n" << allPaths[i];
            }
        } else {
            for (int i = 0; i < 10 && i < allPaths.size(); i++) {
                s << "\n" << allPaths[i].toNiceString(*tt);
            }
        }
    }

    s << "\n\nOf the people going by Västtrafik, this was the distribution of the number of stops they could walk to "
         "from "
         "home";
    if (opts != nullptr) s << " (less than " << opts->moveableDistance << "m)";
    s << ":\n";
    for (auto [nStops, nPpl] : distNumberOfStartStops) {
        auto postSpace = nStops < 10 ? " " : "";

        int nPreSpace = 10;
        for (int x = nPpl; x /= 10; nPreSpace--)
            ;
        std::stringstream preSpace;
        for (int x = nPreSpace; x; x--) preSpace << " ";

        s << nStops << postSpace << ": " << preSpace.str() << nPpl << "\n";
    }

    s << "\nOf the people going by Västtrafik, this was the distribution of the number of stops they could walk to the "
         "destination from";
    if (opts != nullptr) s << " (less than " << opts->moveableDistance << "m)";
    s << ":\n";
    for (auto [nStops, nPpl] : distNumberOfEndStops) {
        auto postSpace = nStops < 10 ? " " : "";

        int nPreSpace = 10;
        for (int x = nPpl; x /= 10; nPreSpace--)
            ;
        std::stringstream preSpace;
        for (int x = nPreSpace; x; x--) preSpace << " ";

        s << nStops << postSpace << ": " << preSpace.str() << nPpl << "\n";
    }

    return s.str();
}
