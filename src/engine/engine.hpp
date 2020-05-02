#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <optional>
#include <filesystem>
#include <vector>
#include <utility>

namespace fs = std::filesystem;

class OpResult {
public:
    bool success;
    std::optional<std::string> value;
    std::optional<std::string> error_msg;
};

class AbstractEngine {
public:
    virtual OpResult get(std::string key) = 0;
    virtual OpResult set(std::string key, std::string value) = 0;
    virtual OpResult drop(std::string key) = 0;
};


struct SegmentFile {
    fs::path path;
    int number;
    bool compressed = false;
    std::streampos length = 0;

    SegmentFile(fs::path p_, int n_, bool c_) : path(p_), number(n_), compressed(c_) {};
};


class SingleFileLogEngine : public AbstractEngine {
public:
    SingleFileLogEngine(fs::path path);
    OpResult get(std::string key);
    OpResult set(std::string key, std::string value);
    OpResult drop(std::string key);
private:
    fs::path dir;
    std::vector<SegmentFile> segments;

    std::fstream current;
    std::map<std::string, std::pair<SegmentFile*, std::streampos>> cache;

    void load_segment(SegmentFile& );
};
