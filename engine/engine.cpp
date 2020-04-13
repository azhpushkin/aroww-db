#include "engine.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <optional>

bool operator==(const OpResult& lhs, const OpResult& rhs) {
    return (
        lhs.success == rhs.success
        and lhs.error_msg == rhs.error_msg
        and lhs.value == rhs.value
    );
}
bool operator!=(const OpResult& lhs, const OpResult& rhs) {
    return !(lhs==rhs);
}

SimpleInMemoryEngine::SimpleInMemoryEngine() {}

OpResult SimpleInMemoryEngine::get(std::string key)
{   
    try {
        return OpResult {true, storage.at(key), std::nullopt};
    } catch (std::out_of_range &e) {
        return OpResult {false, std::nullopt, "Key missing"};
    }
}

OpResult SimpleInMemoryEngine::set(std::string key, std::string value)
{
    storage[key] = value;
    return OpResult {true, std::nullopt, std::nullopt};
}

OpResult SimpleInMemoryEngine::drop(std::string key)
{
    storage.erase(key);
    return OpResult {true, std::nullopt, std::nullopt};
}
