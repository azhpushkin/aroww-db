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


class SingleFileLogEngine : public AbstractEngine {
public:
    SingleFileLogEngine(fs::path path);
    OpResult get(std::string key);
    OpResult set(std::string key, std::string value);
    OpResult drop(std::string key);
private:
    fs::path dir;
    std::vector<std::shared_ptr<SegmentFile>> segments;

    std::fstream current;
    std::map<std::string, KeyPosition> cache;

    void load_segment(std::shared_ptr<SegmentFile> segment);
};

