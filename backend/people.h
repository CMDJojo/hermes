#pragma once

#include <algorithm>
#include <boost/functional/hash.hpp>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "csvLoader.h"
#include "gauss-kruger/gausskruger.h"

struct DMSCoord;

struct MeterCoord {
    int x, y;

    MeterCoord() = default;

    MeterCoord(int x, int y) : x(x), y(y) {}

    [[nodiscard]] float distanceTo(MeterCoord other) const;

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

    float distanceToWork() const {
        return home_coord.distanceTo(work_coord);
    }

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
    People(const std::string& rawPersonPath, bool alsoBuildIndex = true);

    void buildIndex();

    static std::vector<MeterCoord> createSquare(
        MeterCoord low, MeterCoord high, int xRes, int yRes,
        const std::function<bool(MeterCoord)>& pred = constfn<bool, MeterCoord>(true));

    static std::vector<MeterCoord> constrainedSquare(
        MeterCoord origin, int dx, int dy, int multiple, int offset,
        const std::function<bool(MeterCoord)>& pred = constfn<bool, MeterCoord>(true));

    static std::vector<MeterCoord> constrainedCircle(MeterCoord origin, int radius, int multiple, int offset);

    std::vector<Person> allPersonsInDomain(const std::vector<MeterCoord>& domain);

    static std::vector<MeterCoord> personCoordsInCircle(MeterCoord origin, int radius);

    static std::vector<MeterCoord> personCoordsInCircle(DMSCoord origin, int radius);

    std::vector<Person> personsInCircle(MeterCoord origin, int radius);

    std::vector<Person> personsInCircle(DMSCoord origin, int radius);

    std::vector<Person> naivePersonsInCircle(MeterCoord origin, int radius);

    static bool euclideanDistanceLEQ(MeterCoord a, MeterCoord b, int64_t d);

    static std::function<bool(MeterCoord)> euclideanPredicate(MeterCoord origin, int d);

    static void test();

    std::vector<Person> people;
    std::unordered_map<MeterCoord, std::vector<Person>> indexedPeople{};

   private:
    std::vector<Person> load(const std::string& rawPersonPath);
};
