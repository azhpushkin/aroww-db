#pragma once

#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <utility>
#include <list>

#include "engine.hpp"


namespace fs = std::filesystem;

class AppendLogEngine;

class SegmentFile {
public:
    int number;
    bool compressed;
    int length = 0;

    SegmentFile(int n_, bool c_) : number(n_), compressed(c_) {};

    fs::path get_path(AppendLogEngine*);
    static std::optional<SegmentFile> parse_path(fs::path);
};

typedef std::pair<std::shared_ptr<SegmentFile>, std::streampos> KeyPosition;


class AppendLogConfiguration {
public:
    fs::path dir_path;
    int max_segment_size;
    AppendLogConfiguration(fs::path p_, int m_): dir_path(p_), max_segment_size(m_) {};
};


class AppendLogEngine : public AbstractEngine {
public:
    AppendLogEngine(AppendLogConfiguration conf);
    OpResult get(std::string key);
    OpResult set(std::string key, std::string value);
    OpResult drop(std::string key);
private:
    AppendLogConfiguration conf;
    fs::path data_dir;
    
    std::list<std::shared_ptr<SegmentFile>> segments;

    std::fstream write_file;
    std::map<std::string, KeyPosition> cache;
    
    void load_segment(std::shared_ptr<SegmentFile> segment);
    void switch_to_new_segment();

    friend class SegmentFile;
};

