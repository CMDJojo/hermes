#pragma once

#include "csvLoader.h"
#include "gauss-kruger/gausskruger.h"

struct Coord {
    double longitude, latitude;

    Coord(double longitude, double latitude) : longitude(longitude), latitude(latitude) {}
};

enum class County : uint32_t {
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

using Municipality = uint16_t;

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
    Coord work_coord;
    County home_county;
    Municipality home_municipality;
    Coord home_coord;

    Person(bool is_female,
           County work_county,
           Municipality work_municipality,
           Coord work_coord,
           County home_county,
           Municipality home_municipality,
           Coord home_coord) : is_female(is_female), work_county(work_county), work_municipality(work_municipality),
                               work_coord(work_coord), home_county(home_county), home_municipality(home_municipality),
                               home_coord(home_coord) {}

    static std::vector<Person> load(const std::string &rawPersonPath) {
        std::vector<RawPerson> rawPersons = csvLoader::load<RawPerson, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t>(
                rawPersonPath);

        std::vector<Person> persons;
        for (auto p : rawPersons) {
            double latitude, longitude;
            gausskruger::SWEREF99TM projection;

            projection.gridToGeodetic(p.XKOORD_Ast, p.YKOORD_Ast, latitude, longitude);
            Coord work_coord = Coord(longitude, latitude);

            projection.gridToGeodetic(p.XKOORD_Bost, p.YKOORD_Bost, latitude, longitude);
            Coord home_coord = Coord(longitude, latitude);

            persons.emplace_back(p.kon - 1, (County) p.Lan_Ast, p.Kommun_Ast, work_coord, (County) p.Lan_Bost,
                                 p.Kommun_Bost, home_coord);
        }
        return persons;
    }
};
