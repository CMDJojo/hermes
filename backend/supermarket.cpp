
#include "supermarket.h"
#include <utility>

namespace supermarket {

    std::vector<Supermarket> Supermarket::load(const std::string &filepath) {
        return csvLoader::load<Supermarket, std::string, double, double>(
                filepath + "/supermarkets.csv");
    }

    Supermarket::Supermarket(std::string name, const double lat, const double lon) : name(std::move(name)) {
        coords = DMSCoord(lat, lon);
    }


}