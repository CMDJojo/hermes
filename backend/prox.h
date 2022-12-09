#pragma once

#include <cmath>
#include <iostream>
#include <numbers>
#include <vector>

#include "gauss-kruger/gausskruger.h"
#include "gtfsTypes.h"
#include "people.h"
#include "routing.h"

using StopCoord = std::pair<DMSCoord, StopId>;

class Prox {
   public:
    Prox(const routing::Timetable& timetable);

    std::vector<std::pair<StopId, double>>
    stopsIDAndDistanceMultipliedWithAFactorWhichInFactIsJustTheWalkSpeedWithinACertainRangeInclusiveButRounded(
        const MeterCoord& coord, uint32_t range, double mysticFactor) const;

    std::vector<std::pair<StopId, double>>
    stopsIDAndDistanceMultipliedWithAFactorWhichInFactIsJustTheWalkSpeedWithinACertainRangeInclusiveButRounded(
        const DMSCoord& coord, uint32_t range, double mysticFactor) const;

    std::vector<std::pair<StopId, double>> stopsAroundDMSCoord(const DMSCoord& coord, double range) const;

    std::vector<std::pair<StopId, double>> stopsAroundMeterCoord(const MeterCoord mCoord, double range) const;

    std::vector<StopCoord> stops;

   private:
    const double earthRadius = 6'371'009;

    double distance(double lat1, double lat2, double lon1, double lon2) const;

    double distance2(double lat1, double lat2, double lon1, double lon2) const;

    static double toRadian(double deg);

    static double meterToDegreeLat(double meters);

    static double meterToDegreeLon(double meters, double lat);

    static bool stopComparator(const StopCoord& lhs, const StopCoord& rhs);
};
