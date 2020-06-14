#pragma once

#include <iostream>
#include <string>
#include <cstdint>
#include <variant>

#define TOMB_MARK -1

class tomb {
public:
    static tomb create();
};
typedef std::variant<std::string, tomb>  string_or_tomb;


void pack_char(std::ostream& ss, const char& c);
void unpack_char(std::istream& ss, char& c);

void pack_string(std::ostream& ss, const std::string& field);
void unpack_string(std::istream& ss, std::string& field);

void pack_int64(std::ostream& ss, const int64_t& field);
void unpack_int64(std::istream& ss, int64_t& field);

void pack_string_or_tomb(std::ostream& ss, const string_or_tomb&);
void unpack_string_or_tomb(std::istream& ss, string_or_tomb&);
