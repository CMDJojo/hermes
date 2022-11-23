#include "people.h"

DMSCoord MeterCoord::toDMS() const {
    gausskruger::SWEREF99TM projection;
    double latitude, longitude;
    projection.gridToGeodetic(x, y, latitude, longitude);
    return {latitude, longitude};
}

MeterCoord DMSCoord::toMeter() const {
    gausskruger::SWEREF99TM projection;
    double x, y;
    projection.geodeticToGrid(latitude, longitude, x, y);
    return {static_cast<int>(x), static_cast<int>(y)};
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

    std::vector<Person> people;

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
    People people("data/raw/Ast_bost.txt");
    std::vector<Person> in_radius = people.findPeople(57.696515, 11.971184, 1000);
    std::cout << in_radius.size() << std::endl;
}
