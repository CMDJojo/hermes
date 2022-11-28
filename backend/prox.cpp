#include "prox.h"

#include <algorithm>
#include <numbers>

using namespace gtfs;

class Prox {
   public:
    Prox(const std::string& stopsPath) {
        stops = Stop::load(stopsPath);
        std::sort(stops.begin(), stops.end(),
                  [](const Stop& lhs, const Stop& rhs) { return stopComparator(lhs, rhs); });
        std::copy_if(stops.begin(), stops.end(), std::back_inserter(filteredStops),
                     [](Stop stop) { return !isStopPoint(stop.stopId); });
    }

    std::vector<Stop> stopsAroundDMSCoord(const DMSCoord& coord, double range) {
        std::vector<Stop> found;

        DMSCoord lowerCoord = {coord.latitude - meterToDegreeLat(range),
                               coord.longitude - meterToDegreeLon(range, coord.latitude)};
        DMSCoord upperCoord = {coord.latitude + meterToDegreeLat(range),
                               coord.longitude + meterToDegreeLon(range, coord.latitude)};

        Stop lower(0, "", lowerCoord.latitude, lowerCoord.longitude, 0, Ignore(), Ignore());
        Stop upper(0, "", upperCoord.latitude, upperCoord.longitude, 0, Ignore(), Ignore());

        auto start = std::lower_bound(filteredStops.begin(), filteredStops.end(), lower, stopComparator);
        auto end = std::next(std::lower_bound(start, filteredStops.end(), upper, stopComparator));

        const double distanceToCompare = pow(range / earthRadius, 2);

        while (start != end) {
            if (distance2(coord.latitude, start->stopLat, coord.longitude, start->stopLon) < distanceToCompare) {
                found.push_back(*start);
            }

            start++;
        }

        return found;
    }

    const double earthRadius = 6'371'009;
    ;  // radius of the earth in meters

    // distance between two coordinates, uses square root.
    double distance(double lat1, double lat2, double lon1, double lon2) {
        double meanLat = (lat1 + lat2) / 2;
        double deltaLat = toRadian(lat2 - lat1);
        double deltaLon = toRadian(lon2 - lon1);

        meanLat = toRadian(meanLat);
        double c = (pow(deltaLat, 2) + pow(cos(meanLat) * (deltaLon), 2));

        return earthRadius * sqrt(c);
    }

    // square distance between two coordinates divided by the radius of the earth.
    double distance2(double lat1, double lat2, double lon1, double lon2) {
        double meanLat = (lat1 + lat2) / 2;
        double deltaLat = toRadian(lat2 - lat1);
        double deltaLon = toRadian(lon2 - lon1);

        meanLat = toRadian(meanLat);
        double c = (pow(deltaLat, 2) + pow(cos(meanLat) * (deltaLon), 2));

        return c;
    }

    std::vector<Stop> stopsAroundMeterCoord(const MeterCoord mCoord, double range) {
        return stopsAroundDMSCoord(mCoord.toDMS(), range);
    }

    std::vector<Stop> naiveStopsAroundDMSCoord(const DMSCoord coord, double range) {
        std::vector<Stop> found;
        const double distanceToCompare = pow(range / earthRadius, 2);
        for (auto stop : filteredStops) {
            if (distance2(coord.latitude, stop.stopLat, coord.longitude, stop.stopLon) < distanceToCompare) {
                found.push_back(stop);
            }
        }

        return found;
    }

    std::vector<Stop> stops;
    std::vector<Stop> filteredStops;

   private:
    static double toRadian(double deg) { return deg * (std::numbers::pi / 180); };

    static double meterToDegreeLat(double meters) { return meters / 111'320; }

    static double meterToDegreeLon(double meters, double lat) { return meters / (111'320 * cos(toRadian(lat))); }

    static bool isStopPoint(StopId stopId) { return stopId % 10000000000000 / 1000000000000 == 2; }

    static bool stopComparator(const Stop& lhs, const Stop& rhs) {
        if (lhs.stopLat < rhs.stopLat) return true;
        if (rhs.stopLat < lhs.stopLat) return false;
        return lhs.stopLon < rhs.stopLon;
    }
};

int main() {
    Prox prox("data/raw");
    DMSCoord coord = {57.707030, 11.967837};  // Brunnsparken

    auto startTime1 = std::chrono::high_resolution_clock::now();
    std::vector<Stop> found1 = prox.stopsAroundDMSCoord(coord, 1000);
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
    for (auto stop : found1) {
        std::cout << "lat: " << stop.stopLat << "  lon: " << stop.stopLon << "  " << stop.stopName << std::endl;
    }

    std::cout << std::endl << "Found2: " << std::endl;
    for (auto stop : found2) {
        std::cout << "lat: " << stop.stopLat << "  lon: " << stop.stopLon << "  " << stop.stopName << std::endl;
    }

    return 0;
}
