#include "gtfsTypes.h"

#include "csvLoader.h"

namespace gtfs {

std::vector<Agency> Agency::load(const std::string& gtfsPath) {
    return csvLoader::load<Agency, AgencyId, std::string, std::string, std::string, std::string, std::string>(
        gtfsPath + "/agency.txt");
}
std::vector<Attribution> Attribution::load(const std::string& gtfsPath) {
    return csvLoader::load<Attribution, TripId, std::string, bool>(gtfsPath + "/attributions.txt");
}
std::vector<Calendar> Calendar::load(const std::string& gtfsPath) {
    return csvLoader::load<Calendar, ServiceId, bool, bool, bool, bool, bool, bool, bool, Date, Date>(gtfsPath +
                                                                                                      "/calendar.txt");
}
std::vector<CalendarDate> CalendarDate::load(const std::string& gtfsPath) {
    return csvLoader::load<CalendarDate, ServiceId, Date, bool>(gtfsPath + "/calendar_dates.txt");
}
std::vector<FeedInfo> FeedInfo::load(const std::string& gtfsPath) {
    return csvLoader::load<FeedInfo, std::string, std::string, std::string, std::string, std::string>(gtfsPath +
                                                                                                      "/feed_info.txt");
}
std::vector<Route> Route::load(const std::string& gtfsPath) {
    return csvLoader::load<Route, RouteId, AgencyId, std::string, std::string, int, std::string>(gtfsPath +
                                                                                                 "/routes.txt");
}
std::vector<Shape> Shape::load(const std::string& gtfsPath) {
    return csvLoader::load<Shape, ShapeId, double, double, int, double>(gtfsPath + "/shapes.txt");
}

std::vector<StopTime> StopTime::load(const std::string& gtfsPath) {
    return csvLoader::load<StopTime, TripId, Time, Time, StopId, int, std::string, int, int, Ignore, bool>(
        gtfsPath + "/stop_times.txt");
}

std::vector<Stop> Stop::load(const std::string& gtfsPath) {
    return csvLoader::load<Stop, StopId, std::string, float, float, int32_t, Ignore, Ignore>(gtfsPath + "/stops.txt");
}

std::vector<Transfer> Transfer::load(const std::string& gtfsPath) {
    return csvLoader::load<Transfer, StopId, StopId, int, int, TripId, TripId>(gtfsPath + "/transfers.txt");
}

std::vector<Trip> Trip::load(const std::string& gtfsPath) {
    return csvLoader::load<Trip, uint64_t, ServiceId, TripId, std::string, int32_t, uint64_t>(gtfsPath + "/trips.txt");
}

#include "functional"
template <typename T>
void testLoad(std::function<std::vector<T>(const std::string&)> loader, const std::string& name) {
    std::cout << "[TEST] Loading type: " << name << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    auto payload = loader("data/raw");
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(stop - start).count();
    auto items = payload.size();
    std::cout << "[TEST] " << items << "items / ";
    std::cout << duration << "μs / ";
    std::cout << (duration / items) << "μs/item";
    std::cout << std::endl;
}

#define TEST_LOAD(name) testLoad<name>(name::load, #name)

void test() {
    auto startAll = std::chrono::high_resolution_clock::now();
    std::cout << "[TEST] Loading all GTFS types" << std::endl;

    TEST_LOAD(Agency);
    TEST_LOAD(Attribution);
    TEST_LOAD(Calendar);
    TEST_LOAD(CalendarDate);
    TEST_LOAD(FeedInfo);
    TEST_LOAD(Route);
    TEST_LOAD(Shape);
    TEST_LOAD(StopTime);
    TEST_LOAD(Stop);
    TEST_LOAD(Transfer);
    TEST_LOAD(Trip);

    auto stopAll = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(stopAll - startAll);
    std::cout << "[TEST] Test finished in " << duration.count() << "ms" << std::endl;
}
}  // namespace gtfs
