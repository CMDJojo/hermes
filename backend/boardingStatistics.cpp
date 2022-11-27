#include "boardingStatistics.h"
#include "csvLoader.h"
#include "gtfsTypes.h"

#include <iostream>

namespace boarding {
    std::unordered_map<StopId, int> rawStats{};

    struct Stat {
        StopId id;
        int passengers;

        Stat(StopId id, int passengers) : id(id), passengers(passengers) {}
    };

    void load(const std::string &file) {
        if (rawStats.empty()) {
            for (Stat stat: csvLoader::load<Stat, StopId, int>(file)) {
                rawStats.emplace(stat.id, stat.passengers);
            }
        } else {
            std::cout << "Boarding::load called but file already loaded" << std::endl;
        }
    }

    bool isImportant(StopId stopId) {
        assert(!rawStats.empty());
        return rawStats.contains(stopId);
    }

    std::unordered_map<StopId, int> getStats() { return rawStats; }
}
