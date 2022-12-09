#include "prox.h"

#include <algorithm>
#include <numbers>
#include <utility>

#include "people.h"

using namespace gtfs;

Prox::Prox(const routing::Timetable& timetable) {
    for (auto& [stopId, stopNode] : timetable.stops) {
        stops.emplace_back(DMSCoord(stopNode.lat, stopNode.lon), stopId);
    }
    std::sort(stops.begin(), stops.end(), [](const auto& a, const auto& b) { return stopComparator(a, b); });
}

std::vector<std::pair<StopId, double>> Prox::stopsAroundDMSCoord(const DMSCoord& coord, double range) const {
    std::vector<std::pair<StopId, double>> found;

    DMSCoord lowerCoord = {coord.latitude - meterToDegreeLat(range),
                           coord.longitude - meterToDegreeLon(range, coord.latitude)};
    DMSCoord upperCoord = {coord.latitude + meterToDegreeLat(range),
                           coord.longitude + meterToDegreeLon(range, coord.latitude)};

    const StopCoord lower(lowerCoord, 0);
    const StopCoord upper(upperCoord, 0);

    auto start = std::lower_bound(stops.begin(), stops.end(), lower, stopComparator);
    auto end = std::next(std::lower_bound(start, stops.end(), upper, stopComparator));

    // const double distanceToCompare = pow(range / earthRadius, 2);

    while (start != end) {
        double dist = distance(coord.latitude, start->first.latitude, coord.longitude, start->first.longitude);
        if (dist < range) {
            found.emplace_back(start->second, dist);
        }

        start++;
    }

    return found;
}

// distance between two coordinates, uses square root.
double Prox::distance(double lat1, double lat2, double lon1, double lon2) const {
    double meanLat = (lat1 + lat2) / 2;
    double deltaLat = toRadian(lat2 - lat1);
    double deltaLon = toRadian(lon2 - lon1);

    meanLat = toRadian(meanLat);
    double c = (pow(deltaLat, 2) + pow(cos(meanLat) * (deltaLon), 2));

    return earthRadius * sqrt(c);
}

// square distance between two coordinates divided by the radius of the earth.
double Prox::distance2(double lat1, double lat2, double lon1, double lon2) const {
    double meanLat = (lat1 + lat2) / 2;
    double deltaLat = toRadian(lat2 - lat1);
    double deltaLon = toRadian(lon2 - lon1);

    meanLat = toRadian(meanLat);
    double c = (pow(deltaLat, 2) + pow(cos(meanLat) * (deltaLon), 2));

    return c;
}

double Prox::toRadian(double deg) { return deg * (std::numbers::pi / 180); };

double Prox::meterToDegreeLat(double meters) { return meters / 111'320; }

double Prox::meterToDegreeLon(double meters, double lat) { return meters / (111'320 * cos(toRadian(lat))); }

bool Prox::stopComparator(const StopCoord& lhs, const StopCoord& rhs) {
    if (lhs.first.latitude < rhs.first.latitude) return true;
    if (rhs.first.latitude < lhs.first.latitude) return false;
    return lhs.first.longitude < rhs.first.longitude;
}

std::vector<std::pair<StopId, double>> Prox::stopsAroundMeterCoord(const MeterCoord mCoord, double range) const {
    return stopsAroundDMSCoord(mCoord.toDMS(), range);
}

std::vector<std::pair<StopId, double>>
Prox::stopsIDAndDistanceMultipliedWithAFactorWhichInFactIsJustTheWalkSpeedWithinACertainRangeInclusiveButRounded(
    const DMSCoord& coord, uint32_t range, double mysticFactor) const {
    std::vector<std::pair<StopId, double>> found;

    DMSCoord lowerCoord = {coord.latitude - meterToDegreeLat(range),
                           coord.longitude - meterToDegreeLon(range, coord.latitude)};
    DMSCoord upperCoord = {coord.latitude + meterToDegreeLat(range),
                           coord.longitude + meterToDegreeLon(range, coord.latitude)};

    const StopCoord lower(lowerCoord, 0);
    const StopCoord upper(upperCoord, 0);

    auto start = std::lower_bound(stops.begin(), stops.end(), lower, stopComparator);
    auto end = std::next(std::lower_bound(start, stops.end(), upper, stopComparator));

    // const double distanceToCompare = pow(range / earthRadius, 2);

    while (start != end) {
        double dist = distance(coord.latitude, start->first.latitude, coord.longitude, start->first.longitude);
        if (dist < range) {
            found.emplace_back(start->second, dist * mysticFactor);
        }

        start++;
    }

    return found;
}

std::vector<std::pair<StopId, double>>
Prox::stopsIDAndDistanceMultipliedWithAFactorWhichInFactIsJustTheWalkSpeedWithinACertainRangeInclusiveButRounded(
    const MeterCoord& coord, uint32_t range, double mysticFactor) const {
    return stopsIDAndDistanceMultipliedWithAFactorWhichInFactIsJustTheWalkSpeedWithinACertainRangeInclusiveButRounded(
        coord.toDMS(), range, mysticFactor);
}

/*
int main() {

    Prox prox("data/raw");
    DMSCoord coord = {57.707030, 11.967837};  // Brunnsparken

    auto startTime1 = std::chrono::high_resolution_clock::now();
    std::vector<std::pair<Stop, double>> found1 = prox.stopsAroundDMSCoord(coord, 1000);
    auto stopTime1 = std::chrono::high_resolution_clock::now();
    auto duration1 = duration_cast<std::chrono::microseconds>(stopTime1 - startTime1).count();
    std::cout << "[stopsAroundDMSCoord] Time taken: " << duration1 << "µs " << found1.size() << " stops found"
              << std::endl;

    auto startTime2 = std::chrono::high_resolution_clock::now();
    std::vector<Stop> found2 = prox.naiveStopsAroundDMSCoord(coord, 1000);
    auto stopTime2 = std::chrono::high_resolution_clock::now();
    auto duration2 = duration_cast<std::chrono::microseconds>(stopTime2 - startTime2).count();
    std::cout << "[naiveStopsAroundDMSCoord] Time taken: " << duration2 << "µs " << found2.size() << " stops found"
              << std::endl;

    std::cout << std::endl << "Found1: " << std::endl;
    for (auto stopPair: found1) {
        std::cout << "lat: " << stopPair.first.stopLat << "  lon: " << stopPair.first.stopLon << "  "
                  << stopPair.first.stopName << "  dist: " << stopPair.second << std::endl;
    }

    std::cout << std::endl << "Found2: " << std::endl;
    for (auto stop: found2) {
        std::cout << "lat: " << stop.stopLat << "  lon: " << stop.stopLon << "  " << stop.stopName << std::endl;
    }

    return 0;
}
*/
