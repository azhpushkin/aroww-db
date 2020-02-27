#pragma once

#include <iostream>
#include <map>

class AbstractEngine {
public:
    virtual std::string get(std::string key) = 0;
    virtual std::string set(std::string key, std::string value) = 0;
    virtual std::string drop(std::string key) = 0;
};

class SimpleInMemoryEngine : public AbstractEngine {
private:
    std::map<std::string, std::string> storage;

public:
    SimpleInMemoryEngine();
    std::string get(std::string key);
    std::string set(std::string key, std::string value);
    std::string drop(std::string key);
};
