#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <utility>
#include <list>
#include <mutex>
#include <optional>
#include <variant>

namespace fs = std::filesystem;

// Memtable
struct memtablecomp {
  bool operator() (const std::string& lhs, const std::string& rhs) const
  {return lhs<rhs;}
};

typedef std::map<std::string, std::optional<std::string>, memtablecomp> MemTable;

MemTable load_memtable(fs::path p);

// Segment
typedef std::map<std::string, int64_t> SegmentIndex;
class Segment;
typedef std::shared_ptr<Segment> SegmentPtr;

class Segment {
public:
    fs::path file_path;
    std::int64_t timestamp;  // timestamp, used for ordering
    std::int64_t keys_amount;
    std::int64_t indexed_keys_amount;
    int64_t index_start;
    
    Segment(fs::path d);

    static SegmentPtr dump_memtable(MemTable& mtbl, fs::path dir, int64_t timestamp, unsigned int index_step);
    static SegmentPtr merge(std::vector<SegmentPtr>, unsigned int index_step);

    std::optional<std::variant<std::string, std::nullptr_t>> lookup(std::string key);

private:
    SegmentIndex index;

};