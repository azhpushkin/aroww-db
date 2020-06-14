#pragma once

#include <variant>
#include <string>

class tomb {
public:
    static tomb create();
};
typedef std::variant<std::string, tomb>  string_or_tomb;
