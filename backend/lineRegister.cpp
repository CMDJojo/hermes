#include "lineRegister.h"

#include <boost/json.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace lineRegister;

LineRegister::LineRegister(const std::string& lineRegisterPath) {
    std::ifstream file(lineRegisterPath);
    
    if (!file.is_open()) {
        std::cerr << "Could not open file " << lineRegisterPath << std::endl;
        return;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    boost::json::object root = boost::json::parse(buffer.str()).as_object();

    auto line = root.at("lines").at("line").as_array();
    
    for (const boost::json::value& value : line) {
        auto object = value.as_object();
        RouteId routeId = value.at("gid").as_int64();
        
        if (object.find("textColorHTML") != object.end() && object.find("backgroundColorHTML") != object.end()) {
            std::string fgColor, bgColor;
            fgColor = value.at("textColorHTML").as_string();
            bgColor = value.at("backgroundColorHTML").as_string();
            lines[routeId] = {"#" + fgColor, "#" + bgColor};
        }
    }
}
