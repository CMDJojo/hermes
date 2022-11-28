#pragma once

#include <cmath>
#include <iostream>
#include <vector>
#include <numbers>

#include "csvLoader.h"
#include "gauss-kruger/gausskruger.h"
#include "gtfsTypes.h"
#include "people.h"

class Prox {
public:
    Prox(const std::string &stopsPath);

    std::vector<std::pair<gtfs::Stop, double>> stopsAroundDMSCoord(const DMSCoord &coord, double range);

    std::vector<std::pair<gtfs::Stop, double>> stopsAroundMeterCoord(const MeterCoord mCoord, double range);

    std::vector<gtfs::Stop> naiveStopsAroundDMSCoord(const DMSCoord coord, double range);

    std::vector<gtfs::Stop> stops;
    std::vector<gtfs::Stop> filteredStops;

private:

    const double earthRadius = 6'371'009;

    double distance(double lat1, double lat2, double lon1, double lon2);

    double distance2(double lat1, double lat2, double lon1, double lon2);

    static double toRadian(double deg);

    static double meterToDegreeLat(double meters);

    static double meterToDegreeLon(double meters, double lat);

    static bool isStopPoint(StopId stopId);

    static bool stopComparator(const gtfs::Stop &lhs, const gtfs::Stop &rhs);
};