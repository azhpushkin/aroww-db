#pragma once

#include <iostream>
#include <map>
#include <optional>

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

class SimpleInMemoryEngine : public AbstractEngine {
private:
    std::map<std::string, std::string> storage;

public:
    SimpleInMemoryEngine();
    OpResult get(std::string key);
    OpResult set(std::string key, std::string value);
    OpResult drop(std::string key);
};
