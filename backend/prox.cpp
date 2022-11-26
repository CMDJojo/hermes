#include "prox.h"
#include <algorithm>
#include <iomanip>
#include <numbers>

using namespace gtfs;



class Prox{

public:

    Prox(const std::string& stopsPath) {
        stops = Stop::load(stopsPath);
        std::sort(stops.begin(), stops.end(), [](const Stop& lhs, const Stop& rhs){return stopComparator(lhs, rhs);});
        std::copy_if(stops.begin(), stops.end(), std::back_inserter(filteredStops), [](Stop stop){return !isStopPoint(stop.stopId);});

    }

    std::vector<Stop> stopsAroundDMSCoord(const DMSCoord& coord, double range){
        std::vector<Stop> found;

        DMSCoord lowerCoord = {coord.latitude- meterToDegreeLat(range), coord.longitude- meterToDegreeLon(range, coord.latitude)};
        DMSCoord upperCoord = {coord.latitude+ meterToDegreeLat(range), coord.longitude+ meterToDegreeLon(range, coord.latitude)};

        Stop lower(0, "", lowerCoord.latitude, lowerCoord.longitude, 0, Ignore(), Ignore());
        Stop upper(0, "", upperCoord.latitude, upperCoord.longitude, 0, Ignore(), Ignore());

        auto start = std::lower_bound(filteredStops.begin(), filteredStops.end(), lower, [](const Stop& lhs, const Stop& rhs){
            if (lhs.stopLat < rhs.stopLat) return true;
            if (lhs.stopLat > rhs.stopLat) return false;
            return lhs.stopLon < rhs.stopLon;
        });

        auto end = std::next(std::lower_bound(start, filteredStops.end(), upper, [](const Stop& lhs, const Stop& rhs){
            if (lhs.stopLat < rhs.stopLat) return true;
            if (lhs.stopLat > rhs.stopLat) return false;
            return lhs.stopLon <= rhs.stopLon;
        }));


        while (start != end){
            //std::cout << "lat: " << start->stopLat << "  lon: " << start->stopLon << "  " << start->stopName << std::endl;
            if (distance(coord.latitude, start->stopLat, coord.longitude, start->stopLon)<range){
                found.push_back(*start);
            }

            start++;
        }

        return found;
    }

    const double earthRadius = 6'371'009; ; //radius of the earth in meters

    double distance (double lat1, double lat2, double lon1, double lon2){
        double meanLat = (lat1+lat2)/2;
        double deltaLat = toRadian(lat2-lat1);
        double deltaLon = toRadian(lon2-lon1);

        meanLat = toRadian(meanLat);
        double c = (pow(deltaLat,2)+pow(cos(meanLat)*(deltaLon),2));


        return earthRadius * sqrt(c);

    }

    std::vector<Stop> stopsAroundAMeterCoord(MeterCoord mCoord, int range) {
        std::vector<Stop> found;
        DMSCoord dmsCoord = mCoord.toDMS();



        return found;
    }



    std::vector<Stop> stops;
    std::vector<Stop> filteredStops;

private:
    static double toRadian (double deg){return deg * (std::numbers::pi/180);};

    static double meterToDegreeLat(double meters){
        return meters / 111'320;
    }

    static double meterToDegreeLon(double meters, double lat){
        return meters / (111'320 * cos(toRadian(lat)));
    }

    static bool isStopPoint(StopId stopId) { return stopId % 10000000000000 / 1000000000000 == 2; }

    static bool stopComparator(const Stop& lhs, const Stop& rhs){
        if (lhs.stopLat < rhs.stopLat) return true;
        if (rhs.stopLat < lhs.stopLat) return false;
        return lhs.stopLon < rhs.stopLon;
    }

};





int main(){

    Prox prox("data/raw");
    DMSCoord coord = {57.707030,11.967837}; // Brunnsparken

    std::vector<Stop> found = prox.stopsAroundDMSCoord(coord, 300);
    std::cout.precision(10);
    std::cout << std::fixed;

    std::cout << std::endl << "Found: " << std::endl;

    for (auto stop : found){
        std::cout << "lat: " << stop.stopLat << "  lon: " << stop.stopLon << "  " << stop.stopName << std::endl;
    }


    return 0;
}