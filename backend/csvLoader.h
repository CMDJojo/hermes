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

const auto haha_it_broke = EXIT_FAILURE;

namespace csvLoader {

struct Time {
    int32_t timestamp;
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

uint64_t parseu64(const std::string& str) { return std::stoul(str); }

int parseInt(const std::string& str) { return std::stoi(str); }

bool parseBool(const std::string& str) { return parseInt(str) % 2 == 1; }

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
    } else if constexpr (std::is_same_v<T, Time>) {
        return parseTime(str);
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
