#pragma once

#include <algorithm>
#include <boost/functional/hash.hpp>
#include <cmath>
#include <iostream>
#include <vector>

#include "csvLoader.h"
#include "gauss-kruger/gausskruger.h"

struct DMSCoord;

struct MeterCoord {
    int x, y;

    MeterCoord() = default;
    MeterCoord(int x, int y) : x(x), y(y) {}

    float distanceTo(MeterCoord other) const;

    bool operator==(const MeterCoord& rhs) const { return x == rhs.x && y == rhs.y; }
    bool operator!=(const MeterCoord& rhs) const { return !(rhs == *this); }
    bool operator<(const MeterCoord& rhs) const;
    bool operator>(const MeterCoord& rhs) const;
    bool operator<=(const MeterCoord& rhs) const;
    bool operator>=(const MeterCoord& rhs) const;

    [[nodiscard]] DMSCoord toDMS() const;
    friend std::ostream& operator<<(std::ostream& os, const MeterCoord& coord);
};

struct DMSCoord {
    double latitude, longitude;
    DMSCoord() = default;
    DMSCoord(double latitude, double longitude) : latitude(latitude), longitude(longitude) {}
    bool operator==(const DMSCoord& rhs) const { return latitude == rhs.latitude && longitude == rhs.longitude; }
    bool operator!=(const DMSCoord& rhs) const { return !(rhs == *this); }
    bool operator<(const DMSCoord& rhs) const;
    bool operator>(const DMSCoord& rhs) const;
    bool operator<=(const DMSCoord& rhs) const;
    bool operator>=(const DMSCoord& rhs) const;

    [[nodiscard]] MeterCoord toMeter() const;
    friend std::ostream& operator<<(std::ostream& os, const DMSCoord& coord);
};

template <>
struct std::hash<MeterCoord> {
    std::size_t operator()(MeterCoord const& c) const noexcept {
        std::size_t res = 0;
        boost::hash_combine(res, c.x);
        boost::hash_combine(res, c.y);
        return res;
    }
};

template <>
struct std::hash<DMSCoord> {
    std::size_t operator()(DMSCoord const& c) const noexcept {
        std::size_t res = 0;
        boost::hash_combine(res, c.latitude);
        boost::hash_combine(res, c.longitude);
        return res;
    }
};

enum class County : int32_t {
    Stockholm = 1,
    Uppsala = 3,
    Sodermanland = 4,
    Ostergotland = 5,
    Jonkoping = 6,
    Kronobergs = 7,
    Kalmar = 8,
    Gotland = 9,
    Blekinge = 10,
    Skane = 12,
    Halland = 13,
    VastraGotaland = 14,
    Varmland = 17,
    Orebro = 18,
    Vastmanland = 19,
    Dalarna = 20,
    Gavleborgs = 21,
    Vasternorrland = 22,
    Jamtland = 23,
    Vasterbotten = 24,
    Norrbotten = 25
};

using Municipality = int16_t;

struct RawPerson {
    int32_t kon;
    int32_t Lan_Ast;
    int32_t Kommun_Ast;
    int32_t XKOORD_Ast;
    int32_t YKOORD_Ast;
    int32_t Lan_Bost;
    int32_t Kommun_Bost;
    int32_t XKOORD_Bost;
    int32_t YKOORD_Bost;

    RawPerson(int32_t kon, int32_t Lan_Ast, int32_t Kommun_Ast, int32_t XKOORD_Ast, int32_t YKOORD_Ast,
              int32_t Lan_Bost, int32_t Kommun_Bost, int32_t XKOORD_Bost, int32_t YKOORD_Bost)
        : kon(kon),
          Lan_Ast(Lan_Ast),
          Kommun_Ast(Kommun_Ast),
          XKOORD_Ast(XKOORD_Ast),
          YKOORD_Ast(YKOORD_Ast),
          Lan_Bost(Lan_Bost),
          Kommun_Bost(Kommun_Bost),
          XKOORD_Bost(XKOORD_Bost),
          YKOORD_Bost(YKOORD_Bost) {}
};

struct Person {
    bool is_female;
    County work_county;
    Municipality work_municipality;
    MeterCoord work_coord;
    County home_county;
    Municipality home_municipality;
    MeterCoord home_coord;

    Person(bool is_female, County work_county, Municipality work_municipality, MeterCoord work_coord,
           County home_county, Municipality home_municipality, MeterCoord home_coord)
        : is_female(is_female),
          work_county(work_county),
          work_municipality(work_municipality),
          work_coord(work_coord),
          home_county(home_county),
          home_municipality(home_municipality),
          home_coord(home_coord) {}
    bool operator==(const Person& rhs) const {
        return is_female == rhs.is_female && work_county == rhs.work_county &&
               work_municipality == rhs.work_municipality && work_coord == rhs.work_coord &&
               home_county == rhs.home_county && home_municipality == rhs.home_municipality &&
               home_coord == rhs.home_coord;
    }
    bool operator!=(const Person& rhs) const { return !(rhs == *this); }
    bool operator<(const Person& rhs) const {
        if (home_county < rhs.home_county) return true;
        if (rhs.home_county < home_county) return false;
        if (home_municipality < rhs.home_municipality) return true;
        if (rhs.home_municipality < home_municipality) return false;
        return home_coord < rhs.home_coord;
    }
    bool operator>(const Person& rhs) const { return rhs < *this; }
    bool operator<=(const Person& rhs) const { return !(rhs < *this); }
    bool operator>=(const Person& rhs) const { return !(*this < rhs); }
};

template <typename T, typename... Args>
static std::function<T(Args...)> constfn(T v) {
    return [v](Args... t) { return v; };
}

class People {
   public:
    People(const std::string& rawPersonPath) {
        people = load(rawPersonPath);
    }

    void buildIndex() {
        for (auto person : people) {
            // dont ask
            indexedPeople.emplace(person.home_coord, std::vector<Person>{}).first->second.push_back(person);
            // haha explanation: emplace returns (entry, bool) where entry is the entry we want to modify
            // so we get it by .first, then .second gives us the vector we want to insert the person into
        }
    }

    static std::vector<MeterCoord> createSquare(
        MeterCoord low, MeterCoord high, int xRes, int yRes,
        const std::function<bool(MeterCoord)>& pred = constfn<bool, MeterCoord>(true)) {
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

    static std::vector<MeterCoord> constrainedSquare(
        MeterCoord origin, int dx, int dy, int multiple, int offset,
        const std::function<bool(MeterCoord)>& pred = constfn<bool, MeterCoord>(true)) {
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

        return createSquare(lf, hf, multiple, multiple, pred);
    }

    static std::vector<MeterCoord> constrainedCircle(MeterCoord origin, int radius, int multiple, int offset) {
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

    static bool euclideanDistanceLEQ(MeterCoord a, MeterCoord b, int64_t d) {
        int64_t dx = a.x - b.x;
        int64_t dy = a.y - b.y;
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

            people.emplace_back(p.kon - 1, (County)p.Lan_Ast, p.Kommun_Ast, work_coord, (County)p.Lan_Bost,
                                p.Kommun_Bost, home_coord);
        }
        return people;
    }
};
