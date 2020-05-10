#include <iostream>
#include <string>
#include <cstdint>
#include <optional>

#define TOMB_MARK -1


void pack_string(std::ostream& ss, const std::string& field);
void unpack_string(std::istream& ss, std::string& field);

void pack_int64(std::ostream& ss, const int64_t& field);
void unpack_int64(std::istream& ss, int64_t& field);


void pack_string_or_tomb(std::ostream& ss, const std::optional<std::string>&);
void unpack_string_or_tomb(std::istream& ss, std::optional<std::string>&);
