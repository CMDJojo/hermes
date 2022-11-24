#include "people.h"

#include <unordered_map>
#include <utility>

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

class People {
   public:
    People(const std::string& rawPersonPath) {
        people = load(rawPersonPath);
        auto compare = [](Person a, Person b) { return a.home_hilbert_index < b.home_hilbert_index; };
        std::sort(people.begin(), people.end(), compare);
    }

    std::vector<Person> findPeople(double latitude, double longitude, int radius) {
        std::vector<Person> people_in_radius;

        gausskruger::SWEREF99TM projection;
        double x, y;
        projection.geodeticToGrid(latitude, longitude, x, y);

        int hilbert_index = xy2d(pow(2, 30), (int)x, (int)y);

        auto compare = [](Person a, Person b) { return a.home_hilbert_index < b.home_hilbert_index; };
        auto lower = std::lower_bound(people.begin(), people.end(), Person(hilbert_index), compare);

        auto it = lower;
        MeterCoord center(x, y);

        while (distance(it->home_coord, center) < radius) {
            people_in_radius.push_back(*it++);
        }
        it = lower - 1;
        while (distance(it->home_coord, center) < radius) {
            people_in_radius.push_back(*it--);
        }

        return people_in_radius;
    }

    void buildIndex() {
        for (auto person : people) {
            // dont ask
            indexedPeople.emplace(person.home_coord, std::vector<Person>{}).first->second.push_back(person);
            // haha explanation: emplace returns (entry, bool) where entry is the entry we want to modify
            // so we get it by .first, then .second gives us the vector we want to insert the person into

            if (person.home_coord.x % 100 != 50 || person.home_coord.y % 100 != 50) {
                std::cout << "Strange coords? " << person.home_coord << std::endl;
            }
        }
    }

    std::vector<MeterCoord> createSquare(MeterCoord low, MeterCoord high, int xRes, int yRes,
                                         std::function<bool(MeterCoord)> pred = constfn<bool, MeterCoord>(true)) {
        std::vector<MeterCoord> ret;
        for (int xi = 0; low.x + xi * xRes <= high.x; xi++) {
            for (int yi = 0; low.y + yi * yRes <= high.y; yi++) {
                MeterCoord c = {low.x + xi * xRes, low.y + yi * yRes};
                if (pred(c)) {
                    ret.push_back(c);
                }
            }
        }
        return ret;
    }

    std::vector<MeterCoord> constrainedSquare(MeterCoord origin, int dx, int dy, int multiple, int offset,
                                              std::function<bool(MeterCoord)> pred = constfn<bool, MeterCoord>(true)) {
        // this func takes a coord, lets say {395, 219} and a size, lets say {50, 50} to expand in each direction
        // and generates a list of all MeterCoords within that square, where each coord is constrained to be an
        // offset from a multiple of any real number
        // for this example, assume multiple=100 offset=50 (ie all numbers ends in "50")

        // first we calculate the lower left coord: {345, 169}. Then we round down to the multiple => {300, 100} and
        // add the offset => {350, 150}. then, we have to check if these coords are too low (outside the bound), and
        // if that is the case, we add a multiple => {350, 250}

        // then we calculate the upper right coord: {445, 269}. Then we round down the nearest multiple => {400, 200}
        // and add the offset => {450, 250}. then, we have to check if these coords are too high (outside the bound),
        // and if that is the case, we subtract a multiple => {350, 250}

        // so, in the end, we get the coord {350, 250}, this is the only coord in {345<=x<=445, 169<=y<=269}
        // which ends in "50", and this is thus the expected answer

        // this works for non-negative values, if either corner is in the negative side, this won't work as expected
        // additionally, this function takes an optional predicate which filters what coords can be in the
        // resulting list

        assert(offset < multiple);

        MeterCoord lr = {origin.x - dx, origin.y - dy};
        MeterCoord lo = {lr.x / multiple * multiple + offset, lr.y / multiple * multiple + offset};
        MeterCoord lf = {lo.x < lr.x ? lo.x + multiple : lo.x, lo.y < lr.y ? lo.y + multiple : lo.y};

        MeterCoord hr = {origin.x + dx, origin.y + dy};
        MeterCoord ho = {hr.x / multiple * multiple + offset, hr.y / multiple * multiple + offset};
        MeterCoord hf = {ho.x > hr.x ? ho.x - multiple : ho.x, ho.y > hr.y ? ho.y - multiple : ho.y};

        return createSquare(lf, hf, multiple, multiple, std::move(pred));
    }

    std::vector<MeterCoord> constrainedCircle(MeterCoord origin, int radius, int multiple, int offset) {
        // same as constrainedSquare but the point must be in a specified radius of the origin
        assert(offset < multiple);
        return constrainedSquare(origin, radius, radius, multiple, offset, euclideanPredicate(origin, radius));
    }

    std::vector<Person> allPersonsInDomain(const std::vector<MeterCoord>& domain) {
        std::vector<Person> res;
        for (auto c : domain) {
            auto search = indexedPeople.find(c);
            if (search != indexedPeople.end()) {
                res.insert(res.end(), search->second.begin(), search->second.end());
            }
        }
        return res;
    }

    std::vector<Person> personsInCircle(MeterCoord origin, int radius) {
        return allPersonsInDomain(constrainedCircle(origin, radius, 100, 50));
    }

    std::vector<Person> personsInCircle(DMSCoord origin, int radius) {
        return personsInCircle(origin.toMeter(), radius);
    }

    std::vector<Person> naivePersonsInCircle(MeterCoord origin, int radius) {
        std::vector<Person> ret;
        for (Person p : people) {
            if (euclideanDistanceLEQ(p.home_coord, origin, radius)) {
                ret.push_back(p);
            }
        }
        return ret;
    }

    template <typename T, typename... Args>
    static std::function<T(Args...)> constfn(T v) {
        return [v](Args... t) { return v; };
    }

    static bool euclideanDistanceLEQ(MeterCoord a, MeterCoord b, int d) {
        int dx = a.x - b.x;
        int dy = a.y - b.y;
        return dx * dx + dy * dy <= d * d;
    }

    static std::function<bool(MeterCoord)> euclideanPredicate(MeterCoord origin, int d) {
        return [origin, d](MeterCoord b) { return euclideanDistanceLEQ(origin, b, d); };
    }

    std::vector<Person> people;
    std::unordered_map<MeterCoord, std::vector<Person>> indexedPeople{};

   private:
    std::vector<Person> load(const std::string& rawPersonPath) {
        std::vector<RawPerson> rawPersons =
            csvLoader::load<RawPerson, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t>(
                rawPersonPath);

        std::vector<Person> people;
        for (auto p : rawPersons) {
            MeterCoord work_coord = MeterCoord(p.XKOORD_Ast, p.YKOORD_Ast);
            MeterCoord home_coord = MeterCoord(p.XKOORD_Bost, p.YKOORD_Bost);
            int64_t home_hilbert_index = xy2d(pow(2, 30), home_coord.x, home_coord.y);

            people.emplace_back(p.kon - 1, (County)p.Lan_Ast, p.Kommun_Ast, work_coord, (County)p.Lan_Bost,
                                p.Kommun_Bost, home_coord, home_hilbert_index);
        }
        return people;
    }

    int distance(MeterCoord a, MeterCoord b) { return (int)sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2)); }
};

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    People people("data/raw/Ast_bost.txt");
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(stop - start).count();
    std::cout << "[Load] Time taken: " << duration << "ms " << people.people.size() << " entries" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    people.buildIndex();
    auto stop2 = std::chrono::high_resolution_clock::now();
    auto duration2 = duration_cast<std::chrono::milliseconds>(stop2 - start2).count();
    std::cout << "[Index] Time taken: " << duration2 << "ms " << people.indexedPeople.size() << " entries ";

    int count = 0;
    for (const auto& entry : people.indexedPeople) {
        count += entry.second.size();
    }

    std::cout << count << " entries in total" << std::endl;

    double latitude = 57.696515;
    double longitude = 11.971184;
    int radius = 100000;

    std::cout << "Running comparison, " << DMSCoord(latitude, longitude) << ", radius=" << radius << "m" << std::endl;

    auto start3 = std::chrono::high_resolution_clock::now();
    auto pplFound = people.personsInCircle(DMSCoord(latitude, longitude), radius);
    auto stop3 = std::chrono::high_resolution_clock::now();
    auto duration3 = duration_cast<std::chrono::nanoseconds>(stop3 - start3).count();
    std::cout << "[personsInCircle] Time taken: " << duration3 << "µs " << pplFound.size() << " ppl found" << std::endl;

    auto start4 = std::chrono::high_resolution_clock::now();
    auto pplFound2 = people.findPeople(latitude, longitude, radius);
    auto stop4 = std::chrono::high_resolution_clock::now();
    auto duration4 = duration_cast<std::chrono::nanoseconds>(stop4 - start4).count();
    std::cout << "[findPeople] Time taken: " << duration4 << "µs " << pplFound2.size() << " ppl found" << std::endl;

    auto start5 = std::chrono::high_resolution_clock::now();
    auto pplFound3 = people.naivePersonsInCircle(DMSCoord(latitude, longitude).toMeter(), radius);
    auto stop5 = std::chrono::high_resolution_clock::now();
    auto duration5 = duration_cast<std::chrono::nanoseconds>(stop5 - start5).count();
    std::cout << "[naivePersonsInCircle] Time taken: " << duration5 << "µs " << pplFound3.size() << " ppl found"
              << std::endl;

    std::vector<Person> in_radius = people.findPeople(latitude, longitude, radius);
    std::cout << in_radius.size() << std::endl;
}
