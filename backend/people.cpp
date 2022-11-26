#include "people.h"

#include <functional>
#include <unordered_map>
#include <utility>
#include <cmath>

DMSCoord MeterCoord::toDMS() const {
    gausskruger::SWEREF99TM projection;
    double latitude, longitude;
    projection.gridToGeodetic(x, y, latitude, longitude);
    return {latitude, longitude};
}

std::ostream& operator<<(std::ostream& os, const MeterCoord& coord) {
    os << "{x:" << coord.x << ",y:" << coord.y << "}";
    return os;
}

float MeterCoord::distanceTo(MeterCoord other) const {
    int64_t dx = this->x - other.x;
    int64_t dy = this->y - other.y;
    return std::sqrt(dx * dx + dy * dy);
}

bool MeterCoord::operator<(const MeterCoord& rhs) const {
    if (x < rhs.x) return true;
    if (rhs.x < x) return false;
    return y < rhs.y;
}
bool MeterCoord::operator>(const MeterCoord& rhs) const { return rhs < *this; }
bool MeterCoord::operator<=(const MeterCoord& rhs) const { return !(rhs < *this); }
bool MeterCoord::operator>=(const MeterCoord& rhs) const { return !(*this < rhs); }

bool DMSCoord::operator<(const DMSCoord& rhs) const {
    if (latitude < rhs.latitude) return true;
    if (rhs.latitude < latitude) return false;
    return longitude < rhs.longitude;
}
bool DMSCoord::operator>(const DMSCoord& rhs) const { return rhs < *this; }
bool DMSCoord::operator<=(const DMSCoord& rhs) const { return !(rhs < *this); }
bool DMSCoord::operator>=(const DMSCoord& rhs) const { return !(*this < rhs); }

MeterCoord DMSCoord::toMeter() const {
    gausskruger::SWEREF99TM projection;
    double x, y;
    projection.geodeticToGrid(latitude, longitude, x, y);
    return {static_cast<int>(x), static_cast<int>(y)};
}

std::ostream& operator<<(std::ostream& os, const DMSCoord& coord) {
    os << "{lat:" << coord.latitude << ",lon:" << coord.longitude << "}";
    return os;
}

/*int main() {
    auto start = std::chrono::high_resolution_clock::now();
    People people("data/raw/Ast_bost.txt");
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "[Load] Time taken: " << duration << "ms " << people.people.size() << " entries" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    people.buildIndex();
    auto stop2 = std::chrono::high_resolution_clock::now();
    auto duration2 = duration_cast<std::chrono::milliseconds>(stop2 - start2).count();
    std::cout << "[Index] Time taken: " << duration2 << "ms " << people.indexedPeople.size() << " entries, ";

    int count = 0;
    for (const auto& entry : people.indexedPeople) {
        count += entry.second.size();
    }

    std::cout << count << " entries in total" << std::endl;

    double latitude = 57.696515;
    double longitude = 11.971184;
    int radius = 5000;

    std::cout << "Running comparison, " << DMSCoord(latitude, longitude) << ", radius=" << radius << "m" << std::endl;

    auto start3 = std::chrono::high_resolution_clock::now();
    auto pplFound = people.personsInCircle(DMSCoord(latitude, longitude), radius);
    auto stop3 = std::chrono::high_resolution_clock::now();
    auto duration3 = duration_cast<std::chrono::microseconds>(stop3 - start3).count();
    std::cout << "[personsInCircle] Time taken: " << duration3 << "µs " << pplFound.size() << " ppl found" << std::endl;

    auto start5 = std::chrono::high_resolution_clock::now();
    auto pplFound3 = people.naivePersonsInCircle(DMSCoord(latitude, longitude).toMeter(), radius);
    auto stop5 = std::chrono::high_resolution_clock::now();
    auto duration5 = duration_cast<std::chrono::microseconds>(stop5 - start5).count();
    std::cout << "[naivePersonsInCircle] Time taken: " << duration5 << "µs " << pplFound3.size() << " ppl found"
              << std::endl;

    if (pplFound.size() == pplFound3.size()) {
        std::cout << "same number of ppl found with naive and personsInCircle" << std::endl;
    } else {
        std::cout << "different number of ppl found with naive and personsInCircle, finding diff..." << std::endl;
        std::vector<Person> diff;
        std::cout << "sorting" << std::endl;
        std::sort(pplFound3.begin(), pplFound3.end());
        std::sort(pplFound.begin(), pplFound.end());
        std::cout << "sorting done" << std::endl;
        std::set_difference(pplFound3.begin(), pplFound3.end(), pplFound.begin(), pplFound.end(),
                            std::inserter(diff, diff.begin()));
        std::cout << "diff done" << std::endl;

        std::for_each(diff.begin(), diff.end(), [&](const Person& item) {
            std::cout << "included in naive but not in strict: home:" << item.home_coord << std::endl;
        });

        std::cout << "Occurred during find: " << DMSCoord(latitude, longitude).toMeter() << ", radius=" << radius << "m"
                  << std::endl;
    }
}*/
