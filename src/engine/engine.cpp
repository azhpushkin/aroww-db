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

// In memory DB
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


// Append log engine

SingleFileLogEngine::SingleFileLogEngine(std::string filename) {
    file.open(filename, std::ios::in | std::ios::out | std::ios::app);
}


OpResult SingleFileLogEngine::get(std::string key)
{   
    std::string s;
    std::string t;
    std::optional<std::string> res = std::nullopt;
    file.clear();
    file.seekg(0, file.beg);
    
    while (std::getline( file, t )) {
        
        size_t pos = t.find("\t");
        
        if (t.substr(0, pos) == key) {
            if (pos+1 == t.length()) res = std::nullopt;
            else res = t.substr(pos+1, t.length());
            
        }
            
    }
    if (res.has_value()) return OpResult {true, res.value(), std::nullopt};
    else return OpResult {false, std::nullopt, "Key missing"};
}

OpResult SingleFileLogEngine::set(std::string key, std::string value)
{
    file.clear();
    file.seekp(0, file.end);
    file << key << "\t" << value << '\n';
    file.flush();
    return OpResult {true, std::nullopt, std::nullopt};
}

OpResult SingleFileLogEngine::drop(std::string key)
{
    file.clear();
     file.seekp(0, file.end);
    file << key << "\t" << std::endl;
    file.flush();
    return OpResult {true, std::nullopt, std::nullopt};
}
