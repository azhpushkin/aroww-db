#include "engine.hpp"
#include <iostream>
#include <memory>

SimpleInMemoryEngine::SimpleInMemoryEngine()
{
}

std::string SimpleInMemoryEngine::get(std::string key)
{
    return storage[key];  // empty string by default
}

std::string SimpleInMemoryEngine::set(std::string key, std::string value)
{
    storage[key] = value;
    return "SET IS DONE";
}

std::string SimpleInMemoryEngine::drop(std::string key)
{
    storage.erase(key);
    return "DROP IS DONE";
}
