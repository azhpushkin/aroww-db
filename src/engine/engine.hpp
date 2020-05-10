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
class DBEngine;

class Segment {
public:
    fs::path file_path;
    std::int64_t timestamp;  // timestamp, used for ordering
    std::int64_t keys_amount;
    std::int64_t indexed_keys_amount;
    int64_t index_start;
    
    Segment(fs::path d);
    static std::optional<std::shared_ptr<Segment>> parse_path(fs::path);
    static std::shared_ptr<Segment> dump_memtable(MemTable& mtbl, fs::path dir);

    std::optional<std::variant<std::string, std::nullptr_t>> lookup(std::string key);

private:
    SegmentIndex index;

};

typedef std::shared_ptr<Segment> SegmentPnt;



class EngineConfiguration {
public:
    fs::path dir_path;
    int max_segment_size;
    EngineConfiguration(fs::path p_, int m_): dir_path(p_), max_segment_size(m_) {};
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
    
    std::list<SegmentPnt> segments;

    std::mutex write_file_mutex;
    MemTable current_memtable;

    void switch_if_needed();
};

