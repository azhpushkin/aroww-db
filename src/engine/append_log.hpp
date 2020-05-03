#pragma once

#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <utility>

#include "engine.hpp"


namespace fs = std::filesystem;

struct SegmentFile {
    fs::path path;
    int number;
    bool compressed = false;
    std::streampos length = 0;

    SegmentFile(fs::path p_, int n_, bool c_) : path(p_), number(n_), compressed(c_) {};
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
    std::vector<std::shared_ptr<SegmentFile>> segments;

    std::fstream current;
    std::map<std::string, KeyPosition> cache;

    void load_segment(std::shared_ptr<SegmentFile> segment);
};

