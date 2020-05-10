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
    std::int64_t timestamp;  // timestamp, used for ordering
    fs::path dir;  // containing directory
    std::int64_t keys_amount;
    std::int64_t index_amount;
    int64_t index_start;
    
    Segment(std::int64_t s, fs::path d);
    static std::optional<Segment> parse_path(fs::path);
    static std::shared_ptr<Segment> dump_memtable(MemTable& mtbl, fs::path dir);

    std::optional<std::string> lookup(std::string key);
    void clear();

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

