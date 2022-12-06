
#pragma once

#include "csvLoader.h"
#include "people.h"

namespace supermarket {

    struct Supermarket {
        Supermarket(std::string name, double lat, double lon);

        std::string name;
        DMSCoord coords{};

        static std::vector<Supermarket> load(const std::string &superMarketPath);
    };

}
