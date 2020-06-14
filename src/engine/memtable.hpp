#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <utility>
#include <mutex>
#include <optional>

#include "common/string_or_tomb.hpp"

namespace fs = std::filesystem;

// Memtable
struct memtablecomp {
    bool operator() (const std::string& lhs, const std::string& rhs) const
    {return lhs<rhs;}
};

class MemTable {
public:
    std::map<std::string, string_or_tomb, memtablecomp> container;
    std::fstream log_file;
    fs::path log_file_path;

    MemTable(fs::path datadir);
    void set_value(std::string key, string_or_tomb value);
    void cleanup();
};
