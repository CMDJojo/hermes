#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>
#include <chrono>

const auto haha_it_broke = EXIT_FAILURE;

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

std::vector<std::string> split(const std::string& str) {
    int i = 0;
    int ptr = 0;
    char c;
    bool instr = false;
    std::vector<std::string> acc;
    while (true) {
        while ((c = str[ptr++]) != ',' && c != 0 && c != '"')
            ;
        if (c == '"') {
            instr = !instr;
            continue;
        }
        if (instr) {
            continue;
        }
        acc.push_back(str.substr(i, ptr - i - 1));
        i = ptr;
        if (c == 0) break;
    }
    return acc;
}

inline uint64_t parseu64(const std::string& str) {
    if (str.empty() || str == "\r") return 0;
    return std::stoull(str);
    static_assert(sizeof(unsigned long long) == sizeof (uint64_t));
}

inline int parseInt(const std::string& str) {
    if (str.empty() || str == "\r") return 0;
    return std::stoi(str);
    static_assert(sizeof(int) == sizeof (int32_t));
}

inline double parseDouble(const std::string& str) {
    if (str.empty() || str == "\r") return 0;
    return std::stod(str);
}

inline float parseFloat(const std::string& str) {
    if (str.empty() || str == "\r") return 0;
    return std::stof(str);
}

inline bool parseBool(const std::string& str) {
    return !parseInt(str);
}

Date parseDate(const std::string& str) {
    int32_t original = std::stoi(str);
    uint16_t year = static_cast<uint16_t>(std::stoi(str.substr(0, 4)));
    uint8_t month = static_cast<uint8_t>(std::stoi(str.substr(4, 2)));
    uint8_t day = static_cast<uint8_t>(std::stoi(str.substr(6, 2)));
    return {original, year, month, day};
}

Time parseTime(const std::string& str) {
    std::stringstream ss(str);
    int32_t h, m, s;
    char c;
    ss >> h >> c >> m >> c >> s;
    return Time{h * 60 * 60 + m * 60 + s};
}

template <typename T>
T parse(const std::string& str) {
    if constexpr (std::is_same_v<T, std::string>) {
        return str;
    } else if constexpr (std::is_same_v<T, bool>) {
        return parseBool(str);
    } else if constexpr (std::is_same_v<T, uint64_t>) {
        return parseu64(str);
    } else if constexpr (std::is_same_v<T, int32_t>) {
        return parseInt(str);
    } else if constexpr (std::is_same_v<T, double>) {
        return parseDouble(str);
    } else if constexpr (std::is_same_v<T, Date>) {
        return parseDate(str);
    } else if constexpr (std::is_same_v<T, Time>) {
        return parseTime(str);
    } else if constexpr (std::is_same_v<T, float>) {
        return parseFloat(str);
    } else if constexpr (std::is_same_v<T, Ignore>) {
        return {};
    } else {
        static_assert(!sizeof(T),
                      "Invalid type in field for csv loader!! (could not print type name sorry, good luck debugging)");
    }
}

template <typename R, typename... Args>
std::vector<R> load(const std::string& path, bool skipHeader = true) {
    std::string line;
    std::ifstream file;
    file.open(path);

    if (!file.is_open()) {
        std::cerr << "Could not open file " << path << std::endl;
        exit(haha_it_broke);
    }

    std::vector<R> acc;

    while (getline(file, line)) {
        if (skipHeader) {
            skipHeader = false;
            continue;
        }

        auto vals = split(line);
        constexpr size_t n = sizeof...(Args);
        if (vals.size() != n) {
            std::cerr << "Line does not contain " << n << " elements: " << line << std::endl;
            exit(haha_it_broke);
        }

        [&]<std::size_t... Idx>(std::index_sequence<Idx...>) { acc.emplace_back(parse<Args>(vals[Idx])...); }
        (std::make_index_sequence<sizeof...(Args)>{});
    }

    return acc;
}
}  // namespace csvLoader
