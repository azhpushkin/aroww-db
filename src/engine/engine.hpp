#pragma once

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



