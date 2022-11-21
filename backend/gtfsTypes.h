#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <chrono>

#include "csvLoader.h"

using AgencyId = uint64_t;
using RouteId = uint64_t;
using ShapeId = uint64_t;
using TripId = uint64_t;
using StopId = uint64_t;
using ServiceId = int32_t;

using namespace csvLoader;

namespace gtfs {

struct Agency {
    AgencyId agencyId;
    std::string agencyName;
    std::string agencyUrl;
    std::string agencyTimezone;
    std::string agencyLang;
    std::string agencyFareURL;
    Agency(AgencyId agencyId, std::string agencyName, std::string agencyUrl, std::string agencyTimezone,
           std::string agencyLang, std::string agencyFareUrl)
        : agencyId(agencyId),
          agencyName(std::move(agencyName)),
          agencyUrl(std::move(agencyUrl)),
          agencyTimezone(std::move(agencyTimezone)),
          agencyLang(std::move(agencyLang)),
          agencyFareURL(std::move(agencyFareUrl)) {}

    static std::vector<Agency> load(const std::string& gtfsPath) {
        return csvLoader::load<Agency, AgencyId, std::string, std::string, std::string, std::string, std::string>(
            gtfsPath + "/agency.txt");
    }
};

struct Attribution {
    TripId tripId;
    std::string organizationName;
    bool isOperator;
    Attribution(TripId tripId, std::string organizationName, bool isOperator)
        : tripId(tripId), organizationName(std::move(organizationName)), isOperator(isOperator) {}

    static std::vector<Attribution> load(const std::string& gtfsPath) {
        return csvLoader::load<Attribution, TripId, std::string, bool>(gtfsPath + "/attributions.txt");
    }
};

struct Calendar {
    ServiceId serviceId;
    bool monday, tuesday, wednesday, thursday, friday, saturday, sunday;
    Date startDate, endDate;
    Calendar(ServiceId serviceId, bool monday, bool tuesday, bool wednesday, bool thursday, bool friday, bool saturday,
             bool sunday, const Date& startDate, const Date& endDate)
        : serviceId(serviceId),
          monday(monday),
          tuesday(tuesday),
          wednesday(wednesday),
          thursday(thursday),
          friday(friday),
          saturday(saturday),
          sunday(sunday),
          startDate(startDate),
          endDate(endDate) {}

    static std::vector<Calendar> load(const std::string& gtfsPath) {
        return csvLoader::load<Calendar, ServiceId, bool, bool, bool, bool, bool, bool, bool, Date, Date>(
            gtfsPath + "/calendar.txt");
    }
};

struct CalendarDate {
    ServiceId serviceId;
    Date date;
    bool exceptionType;

    CalendarDate(ServiceId service_id, Date date, bool exception_type)
        : serviceId(service_id), date(date), exceptionType(exception_type) {}

    static std::vector<CalendarDate> load(const std::string& gtfsPath) {
        return csvLoader::load<CalendarDate, ServiceId, Date, bool>(gtfsPath + "/calendar_dates.txt");
    }
};

struct FeedInfo {
    std::string feedId, feedPublisherName, feedPublisherUrl, feedLang, feedVersion;
    FeedInfo(std::string feedId, std::string feedPublisherName, std::string feedPublisherUrl, std::string feedLang,
             std::string feedVersion)
        : feedId(std::move(feedId)),
          feedPublisherName(std::move(feedPublisherName)),
          feedPublisherUrl(std::move(feedPublisherUrl)),
          feedLang(std::move(feedLang)),
          feedVersion(std::move(feedVersion)) {}

    static std::vector<FeedInfo> load(const std::string& gtfsPath) {
        return csvLoader::load<FeedInfo, std::string, std::string, std::string, std::string, std::string>(
            gtfsPath + "/feed_info.txt");
    }
};

struct Route {
    RouteId routeId;
    AgencyId agencyId;
    std::string routeShortName, routeLongName;
    int routeType;
    std::string routeDesc;
    Route(RouteId routeId, AgencyId agencyId, std::string routeShortName, std::string routeLongName, int routeType,
          std::string routeDesc)
        : routeId(routeId),
          agencyId(agencyId),
          routeShortName(std::move(routeShortName)),
          routeLongName(std::move(routeLongName)),
          routeType(routeType),
          routeDesc(std::move(routeDesc)) {}

    static std::vector<Route> load(const std::string& gtfsPath) {
        return csvLoader::load<Route, RouteId, AgencyId, std::string, std::string, int, std::string>(gtfsPath +
                                                                                                     "/routes.txt");
    }
};

struct Shape {
    ShapeId shapeId;
    double shapePtLat, shapePtLon;
    int shapePtSequence;
    double shapeDistTravelled;
    Shape(ShapeId shapeId, double shapePtLat, double shapePtLon, int shapePtSequence, double shapeDistTravelled)
        : shapeId(shapeId),
          shapePtLat(shapePtLat),
          shapePtLon(shapePtLon),
          shapePtSequence(shapePtSequence),
          shapeDistTravelled(shapeDistTravelled) {}

    static std::vector<Shape> load(const std::string& gtfsPath) {
        return csvLoader::load<Shape, ShapeId, double, double, int, double>(gtfsPath + "/shapes.txt");
    }
};

struct StopTime {
    TripId tripId;
    Time arrivalTime;
    Time departureTime;
    StopId stopId;
    int32_t stopSequence;
    std::string stopHeadsign;
    int32_t pickupType;
    int32_t dropOffType;
    Ignore shapeDistTravelled;
    bool timepoint;

    StopTime(TripId trip_id, const Time& arrival_time, const Time& departure_time, StopId stop_id, int stop_sequence,
             std::string stop_headsign, int pickup_type, int drop_off_type, const Ignore& shape_dist_travelled,
             bool timepoint)
        : tripId(trip_id),
          arrivalTime(arrival_time),
          departureTime(departure_time),
          stopId(stop_id),
          stopSequence(stop_sequence),
          stopHeadsign(std::move(stop_headsign)),
          pickupType(pickup_type),
          dropOffType(drop_off_type),
          shapeDistTravelled(shape_dist_travelled),
          timepoint(timepoint) {}

    static std::vector<StopTime> load(const std::string& gtfsPath) {
        return csvLoader::load<StopTime, TripId, Time, Time, StopId, int, std::string, int, int, Ignore, bool>(
            gtfsPath + "/stop_times.txt");
    }
};

struct Stop {
    StopId stopId;
    std::string stopName;
    float stopLat;
    float stopLon;
    int32_t locationType;
    Ignore parentStation;
    Ignore platformCode;

    Stop(StopId stop_id, std::string  stop_name, float stop_lat, float stop_lon, int32_t location_type,
         const Ignore& parent_station, const Ignore& platform_code)
        : stopId(stop_id),
          stopName(std::move(stop_name)),
          stopLat(stop_lat),
          stopLon(stop_lon),
          locationType(location_type),
          parentStation(parent_station),
          platformCode(platform_code) {}

    static std::vector<Stop> load(const std::string& gtfsPath) {
        return csvLoader::load<Stop, StopId, std::string, float, float, int32_t, Ignore, Ignore>(gtfsPath +
                                                                                                   "/stops.txt");
    }
};

struct Transfer {
    StopId fromStopId, toStopId;
    int transferType;
    int minTransferTime;
    TripId fromTripId, toTripId;

    Transfer(StopId fromStopId, StopId toStopId, int transferType, int minTransferTime, TripId fromTripId,
             TripId toTripId)
        : fromStopId(fromStopId),
          toStopId(toStopId),
          transferType(transferType),
          minTransferTime(minTransferTime),
          fromTripId(fromTripId),
          toTripId(toTripId) {}

    static std::vector<Transfer> load(const std::string& gtfsPath) {
        return csvLoader::load<Transfer, StopId, StopId, int, int, TripId, TripId>(gtfsPath + "/transfers.txt");
    }
};

struct Trip {
    uint64_t routeId;
    ServiceId serviceId;
    TripId tripId;
    std::string tripHeadsign;
    int32_t directionId;
    uint64_t shapeId;

    Trip(uint64_t route_id, ServiceId service_id, TripId trip_id, std::string trip_headsign, int32_t direction_id,
         uint64_t shape_id)
        : routeId(route_id),
          serviceId(service_id),
          tripId(trip_id),
          tripHeadsign(std::move(trip_headsign)),
          directionId(direction_id),
          shapeId(shape_id) {}

    static std::vector<Trip> load(const std::string& gtfsPath) {
        return csvLoader::load<Trip, uint64_t, ServiceId, TripId, std::string, int32_t, uint64_t>(gtfsPath +
                                                                                                  "/trips.txt");
    }
};

void test() {
    std::string p = "data/raw";

    auto startAll = std::chrono::high_resolution_clock::now();
    std::cout << "[TEST] Loading all GTFS types" << std::endl;
    {
        std::cout << "Loading Agency...";
        auto start = std::chrono::high_resolution_clock::now();
        auto payload = Agency::load(p);
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << " μs (" << payload.size() << " elements)" << std::endl;
    }

    {
        std::cout << "Loading Attribution...";
        auto start = std::chrono::high_resolution_clock::now();
        auto payload = Attribution::load(p);
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << " μs (" << payload.size() << " elements)" << std::endl;
    }

    {
        std::cout << "Loading Calendar...";
        auto start = std::chrono::high_resolution_clock::now();
        auto payload = Calendar::load(p);
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << " μs (" << payload.size() << " elements)" << std::endl;
    }

    {
        std::cout << "Loading CalendarDate...";
        auto start = std::chrono::high_resolution_clock::now();
        auto payload = CalendarDate::load(p);
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << " μs (" << payload.size() << " elements)" << std::endl;
    }

    {
        std::cout << "Loading FeedInfo...";
        auto start = std::chrono::high_resolution_clock::now();
        auto payload = FeedInfo::load(p);
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << " μs (" << payload.size() << " elements)" << std::endl;
    }

    {
        std::cout << "Loading Route...";
        auto start = std::chrono::high_resolution_clock::now();
        auto payload = Route::load(p);
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << " μs (" << payload.size() << " elements)" << std::endl;
    }

    {
        std::cout << "Loading Shape...";
        auto start = std::chrono::high_resolution_clock::now();
        auto payload = Shape::load(p);
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << " μs (" << payload.size() << " elements)" << std::endl;
    }

    {
        std::cout << "Loading StopTime...";
        auto start = std::chrono::high_resolution_clock::now();
        auto payload = StopTime::load(p);
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << " μs (" << payload.size() << " elements)" << std::endl;
    }

    {
        std::cout << "Loading Transfer...";
        auto start = std::chrono::high_resolution_clock::now();
        auto payload = Transfer::load(p);
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << " μs (" << payload.size() << " elements)" << std::endl;
    }

    {
        std::cout << "Loading Trip...";
        auto start = std::chrono::high_resolution_clock::now();
        auto payload = Trip::load(p);
        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << duration.count() << " μs (" << payload.size() << " elements)" << std::endl;
    }

    auto stopAll = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(stopAll - startAll);
    std::cout << "[TEST] Test finished in " << duration.count() << "ms" << std::endl;
}

}  // namespace gtfs
