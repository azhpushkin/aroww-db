#include <iostream>
#include <cstdint>

#include "serialization.hpp"

void pack_char(std::ostream& ss, const char& c) {
    ss.write(&c, 1);
}
void unpack_char(std::istream& ss, char& c) {
    ss.read(&c, 1);
}


void pack_string(std::ostream& ss, const std::string& field) {
    int64_t size = (int64_t) field.size();
    pack_int64(ss, size);
    ss.write(field.c_str(), field.size());
}


void unpack_string(std::istream& ss, std::string& field) {
    int64_t str_size;
    unpack_int64(ss, str_size);

    field.resize(str_size, '\0');
    ss.read(&field[0], str_size);
}


void pack_int64(std::ostream& ss, const int64_t& field) {
    ss.write((char*) &(field), sizeof(int64_t));
}

void unpack_int64(std::istream& ss, int64_t& field) {
    ss.read((char*)(&field), sizeof(int64_t));
}


void pack_string_or_tomb(std::ostream& ss, const string_or_tomb& field) {
    if (std::holds_alternative<std::string>(field)) {
        pack_string(ss, std::get<std::string>(field));
    } else {
        pack_int64(ss, TOMB_MARK);
    }
}
void unpack_string_or_tomb(std::istream& ss, string_or_tomb& field) {
    int64_t str_size;
    unpack_int64(ss, str_size);

    if (str_size == TOMB_MARK) {
        tomb t{};
        field = t;
    } else {
        std::string value(str_size, '\0');
        ss.read(&value[0], str_size);
        field = value;
    }
}
