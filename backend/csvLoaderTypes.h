#pragma once
#include <cstdint>

namespace csvLoader {
struct Time {
    int32_t timestamp;
};

struct Date {
    int32_t original;
    uint16_t year;
    uint8_t month;
    uint8_t day;
};

struct Ignore {};
}
