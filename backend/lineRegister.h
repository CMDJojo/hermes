#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include "gtfsTypes.h"

namespace lineRegister {

struct Line {
    std::string fgColor = "#006C93";
    std::string bgColor = "#FFFFFF";

    Line() = default;
    Line(std::string fg_color, std::string bg_color) : fgColor(std::move(fg_color)), bgColor(std::move(bg_color)) {}
};

}  // namespace lineRegister

struct LineRegister {
    std::unordered_map<RouteId, lineRegister::Line> lines;

    explicit LineRegister(const std::string& lineRegisterPath);
};

