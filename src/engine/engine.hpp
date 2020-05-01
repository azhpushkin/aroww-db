#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <optional>
#include <filesystem>


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


class SingleFileLogEngine : public AbstractEngine {
private:
    std::fstream file;
    std::map<std::string, std::streampos> cache;

public:
    SingleFileLogEngine(std::filesystem::path path);
    OpResult get(std::string key);
    OpResult set(std::string key, std::string value);
    OpResult drop(std::string key);
};
