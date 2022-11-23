#pragma once

#include "csvLoader.h"
#include "hilbert/hilbert.h"
#include "gauss-kruger/gausskruger.h"
#include <boost/functional/hash.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

struct DMSCoord;

struct MeterCoord {
    int x, y;

    MeterCoord() = default;
    MeterCoord(int x, int y) : x(x), y(y) {}
    bool operator==(const MeterCoord& rhs) const { return x == rhs.x && y == rhs.y; }
    bool operator!=(const MeterCoord& rhs) const { return !(rhs == *this); }

    [[nodiscard]] DMSCoord toDMS() const;
};

struct DMSCoord {
    double  latitude,longitude;
    DMSCoord() = default;
    DMSCoord( double latitude,double longitude):  latitude(latitude),longitude(longitude) {}
    bool operator==(const DMSCoord& rhs) const { return latitude == rhs.latitude && longitude == rhs.longitude; }
    bool operator!=(const DMSCoord& rhs) const { return !(rhs == *this); }
    [[nodiscard]] MeterCoord toMeter() const;
};

template <> struct std::hash<MeterCoord> {
    std::size_t operator()(MeterCoord const& c) const noexcept {
        std::size_t res = 0;
        boost::hash_combine(res, c.x);
        boost::hash_combine(res, c.y);
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

    RawPerson(int32_t kon,
              int32_t Lan_Ast,
              int32_t Kommun_Ast,
              int32_t XKOORD_Ast,
              int32_t YKOORD_Ast,
              int32_t Lan_Bost,
              int32_t Kommun_Bost,
              int32_t XKOORD_Bost,
              int32_t YKOORD_Bost) : kon(kon), Lan_Ast(Lan_Ast), Kommun_Ast(Kommun_Ast), XKOORD_Ast(XKOORD_Ast),
                                     YKOORD_Ast(YKOORD_Ast), Lan_Bost(Lan_Bost), Kommun_Bost(Kommun_Bost),
                                     XKOORD_Bost(XKOORD_Bost), YKOORD_Bost(YKOORD_Bost) {}
};

struct Person {
    bool is_female;
    County work_county;
    Municipality work_municipality;
    MeterCoord work_coord;
    County home_county;
    Municipality home_municipality;
    MeterCoord home_coord;
    int64_t home_hilbert_index;

    Person(int home_hilbert_index) : home_hilbert_index(home_hilbert_index) {}

    Person(bool is_female,
           County work_county,
           Municipality work_municipality, MeterCoord work_coord,
           County home_county,
           Municipality home_municipality, MeterCoord home_coord,
           int home_hilbert_index) : is_female(is_female), work_county(work_county),
                                     work_municipality(work_municipality),
                                     work_coord(work_coord), home_county(home_county),
                                     home_municipality(home_municipality),
                                     home_coord(home_coord), home_hilbert_index(home_hilbert_index) {}
};
