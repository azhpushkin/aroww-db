#include "engine.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <optional>


// Append log engine

SingleFileLogEngine::SingleFileLogEngine(std::filesystem::path path) {
    std::filesystem::path file_path = path / "db.txt";
    file.open(file_path.string(), std::ios::in | std::ios::out | std::ios::app);
}


OpResult SingleFileLogEngine::get(std::string key)
{   
    std::string s;
    std::string t;
    std::optional<std::string> res = std::nullopt;
    file.clear();
    if (cache.find(key) != cache.end()) {
        file.seekg(cache.at(key), file.beg);
    } else {
        file.seekg(0, file.beg);
    }
    
    
    while (std::getline( file, t )) {
        
        size_t pos = t.find("\v");
        
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
    cache[key] = file.tellp();
    file << key << "\v" << value << '\n';
    file.flush();
    return OpResult {true, std::nullopt, std::nullopt};
}

OpResult SingleFileLogEngine::drop(std::string key)
{
    file.clear();
    file.seekp(0, file.end);
    cache[key] = file.tellp();
    file << key << "\v" << std::endl;
    file.flush();
    return OpResult {true, std::nullopt, std::nullopt};
}
