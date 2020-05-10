#include <iostream>
#include <cstdint>
#include <optional>

#include "serialization.hpp"

using namespace std;

void pack_string(ostream& ss, const string& field) {
    int64_t size = (int64_t) field.size();
    pack_int64(ss, size);
    ss.write(field.c_str(), field.size());
}


void unpack_string(istream& ss, string& field) {
    int64_t str_size;
    unpack_int64(ss, str_size);

    field.resize(str_size, '\0');
    ss.read(&field[0], str_size);
}


void pack_int64(ostream& ss, const int64_t& field) {
    ss.write((char*) &(field), sizeof(int64_t));
}

void unpack_int64(istream& ss, int64_t& field) {
    ss.read((char*)(&field), sizeof(int64_t));
}


void pack_string_or_tomb(ostream& ss, const optional<string>& field) {
    if (field.has_value()) {
        pack_string(ss, field.value());
    } else {
        pack_int64(ss, TOMB_MARK);
    }
}
void unpack_string_or_tomb(istream& ss, optional<string>& field) {
    int64_t str_size;
    unpack_int64(ss, str_size);

    if (str_size == TOMB_MARK) {
        field.reset();
    } else {
        std::string value(str_size, '\0');
        ss.read(&value[0], str_size);
        field = value;
    }
}
