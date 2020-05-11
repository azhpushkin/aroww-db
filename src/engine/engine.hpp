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

#include "interface.hpp"
#include "network/messages.hpp"


namespace fs = std::filesystem;

struct memtablecomp {
  bool operator() (const std::string& lhs, const std::string& rhs) const
  {return lhs<rhs;}
};

typedef std::map<std::string, std::optional<std::string>, memtablecomp> MemTable;
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

    static std::optional<SegmentPtr> parse_path(fs::path);
    static SegmentPtr dump_memtable(MemTable& mtbl, fs::path dir, int index_step);
    static SegmentPtr merge(std::vector<SegmentPtr>, int index_step);

    std::optional<std::variant<std::string, std::nullptr_t>> lookup(std::string key);

private:
    SegmentIndex index;

};



class EngineConfiguration {
public:
    fs::path dir_path;
    int index_step;
    int max_segment_size;

    EngineConfiguration(fs::path dir, int index_step, int max_segment_size);

    static int DEFAULT_MAX_SEGMENT_SIZE;
    static int DEFAULT_INDEX_STEP;
};


class DBEngine : public AbstractEngine {
public:
    DBEngine(EngineConfiguration conf);
    std::unique_ptr<Message> get(std::string key);
    std::unique_ptr<Message> set(std::string key, std::string value);
    std::unique_ptr<Message> drop(std::string key);
private:
    EngineConfiguration conf;
    fs::path data_dir;
    
    std::list<SegmentPtr> segments;

    std::mutex write_file_mutex;
    MemTable current_memtable;

    void switch_if_needed();
};

