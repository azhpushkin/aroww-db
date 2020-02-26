#pragma once

#include <iostream>

enum EngineResult {
    OK,
    ERROR
};

class AbstractEngine {
public:
    virtual EngineResult get(std::string key) = 0;
    virtual EngineResult set(std::string key, std::string value) = 0;
    virtual EngineResult drop(std::string key) = 0;
};

class SimpleInMemoryEngine : public AbstractEngine {
public:
    EngineResult get(std::string key);
    EngineResult set(std::string key, std::string value);
    EngineResult drop(std::string key);
};
