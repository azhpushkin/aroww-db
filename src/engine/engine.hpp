#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <utility>
#include <list>
#include <mutex>

#include "interface.hpp"


namespace fs = std::filesystem;

struct memtablecomp {
  bool operator() (const std::string& lhs, const std::string& rhs) const
  {return lhs<rhs;}
};

typedef std::map<std::string, std::string, memtablecomp> MemTable;
typedef std::map<std::string, int> SegmentIndex;
class DBEngine;

class Segment {
public:
    std::int64_t number;  // timestamp, used for ordering
    fs::path dir;  // containing directory
    
    Segment(std::int64_t s, fs::path d);
    Segment(std::int64_t s, fs::path d, SegmentIndex&& i);
    static std::optional<Segment> parse_path(fs::path);
    static Segment dump_memtable(MemTable& mtbl, fs::path dir);

    std::optional<std::string> lookup(std::string key);
    void clear();

private:
    std::map<std::string, int> index;

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
    OpResult get(std::string key);
    OpResult set(std::string key, std::string value);
    OpResult drop(std::string key);
private:
    EngineConfiguration conf;
    fs::path data_dir;
    
    std::list<SegmentPnt> segments;

    std::mutex write_file_mutex;
    MemTable current_memtable;

    void switch_if_needed();
};

