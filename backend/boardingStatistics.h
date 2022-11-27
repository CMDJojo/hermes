#pragma once

#include <unordered_map>
#include "gtfsTypes.h"

namespace boarding {
    std::unordered_map<StopId, int> getStats();
    void load(const std::string& path);
    bool isImportant(StopId stopId);
}
