#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "csvLoader.h"

using TripId = uint64_t;
using StopId = uint64_t;
using ServiceId = int32_t;

using namespace csvLoader;

namespace gtfs {

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

struct CalendarDate {
    ServiceId serviceId;
    int32_t date;
    bool exceptionType;

    CalendarDate(ServiceId service_id, int32_t date, bool exception_type)
        : serviceId(service_id), date(date), exceptionType(exception_type) {}

    static std::vector<CalendarDate> load(const std::string& gtfsPath) {
        return csvLoader::load<CalendarDate, ServiceId, int32_t, bool>(gtfsPath + "/calendar_dates.txt");
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

struct Stop {
    StopId stopId;
    std::string stopName;
    Ignore stopLat;
    Ignore stopLon;
    int32_t locationType;
    Ignore parentStation;
    Ignore platformCode;

    Stop(StopId stop_id, const std::string& stop_name, const Ignore& stop_lat, const Ignore& stop_lon,
         int32_t location_type, const Ignore& parent_station, const Ignore& platform_code)
        : stopId(stop_id),
          stopName(stop_name),
          stopLat(stop_lat),
          stopLon(stop_lon),
          locationType(location_type),
          parentStation(parent_station),
          platformCode(platform_code) {}

    static std::vector<Stop> load(const std::string& gtfsPath) {
        return csvLoader::load<Stop, StopId, std::string, Ignore, Ignore, int32_t, Ignore, Ignore>(gtfsPath +
                                                                                                   "/stops.txt");
    }
};

}  // namespace gtfs