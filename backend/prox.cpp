#include "prox.h"

#include <algorithm>
#include <numbers>
#include <utility>
#include "people.h"

using namespace gtfs;



Prox::Prox(const std::string& stopsPath) {
    stops = Stop::load(stopsPath);
    std::sort(stops.begin(), stops.end(),
              [](const Stop& lhs, const Stop& rhs) { return stopComparator(lhs, rhs); });
    std::copy_if(stops.begin(), stops.end(), std::back_inserter(filteredStops),
                 [](Stop stop) { return !isStopPoint(stop.stopId); });
}

std::vector<std::pair<Stop, double>> Prox::stopsAroundDMSCoord(const DMSCoord& coord, double range) {
    std::vector<std::pair<Stop, double>> found;

    DMSCoord lowerCoord = {coord.latitude - meterToDegreeLat(range),
                           coord.longitude - meterToDegreeLon(range, coord.latitude)};
    DMSCoord upperCoord = {coord.latitude + meterToDegreeLat(range),
                           coord.longitude + meterToDegreeLon(range, coord.latitude)};

    Stop lower(0, "", lowerCoord.latitude, lowerCoord.longitude, 0, Ignore(), Ignore());
    Stop upper(0, "", upperCoord.latitude, upperCoord.longitude, 0, Ignore(), Ignore());

    auto start = std::lower_bound(filteredStops.begin(), filteredStops.end(), lower, stopComparator);
    auto end = std::next(std::lower_bound(start, filteredStops.end(), upper, stopComparator));

    //const double distanceToCompare = pow(range / earthRadius, 2);

    while (start != end) {
        double dist = distance(coord.latitude, start->stopLat, coord.longitude, start->stopLon);
        if (dist < range) {
            found.emplace_back(*start, dist);
        }

        start++;
    }

    return found;
}




  // radius of the earth in meters

// distance between two coordinates, uses square root.
double Prox::distance(double lat1, double lat2, double lon1, double lon2) {
    double meanLat = (lat1 + lat2) / 2;
    double deltaLat = toRadian(lat2 - lat1);
    double deltaLon = toRadian(lon2 - lon1);

    meanLat = toRadian(meanLat);
    double c = (pow(deltaLat, 2) + pow(cos(meanLat) * (deltaLon), 2));

    return earthRadius * sqrt(c);
}

// square distance between two coordinates divided by the radius of the earth.
double Prox::distance2(double lat1, double lat2, double lon1, double lon2) {
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

bool Prox::isStopPoint(StopId stopId) { return stopId % 10000000000000 / 1000000000000 == 2; }

bool Prox::stopComparator(const gtfs::Stop& lhs, const gtfs::Stop& rhs) {
    if (lhs.stopLat < rhs.stopLat) return true;
    if (rhs.stopLat < lhs.stopLat) return false;
    return lhs.stopLon < rhs.stopLon;
}

std::vector<std::pair<Stop, double>> Prox::stopsAroundMeterCoord(const MeterCoord mCoord, double range) {
    return stopsAroundDMSCoord(mCoord.toDMS(), range);
}


std::vector<Stop> Prox::naiveStopsAroundDMSCoord(const DMSCoord coord, double range) {
    std::vector<Stop> found;
    const double distanceToCompare = pow(range / earthRadius, 2);
    for (auto stop : filteredStops) {
        if (distance2(coord.latitude, stop.stopLat, coord.longitude, stop.stopLon) < distanceToCompare) {
            found.push_back(stop);
        }
    }

    return found;
}




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
    for (auto stopPair : found1) {
        std::cout << "lat: " << stopPair.first.stopLat << "  lon: " << stopPair.first.stopLon << "  " << stopPair.first.stopName << "  dist: " << stopPair.second  <<std::endl;
    }

    std::cout << std::endl << "Found2: " << std::endl;
    for (auto stop : found2) {
        std::cout << "lat: " << stop.stopLat << "  lon: " << stop.stopLon << "  " << stop.stopName << std::endl;
    }

    return 0;
}
