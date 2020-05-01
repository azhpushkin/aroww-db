#include "engine.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <optional>


// Append log engine

SingleFileLogEngine::SingleFileLogEngine(std::filesystem::path path) {
    std::filesystem::path file_path = path / "db.txt";
    file.open(file_path.string(), std::ios::in | std::ios::out | std::ios::app);

    std::string t, key;
    auto file_pos = file.tellg();
    while (std::getline( file, t )) {
        size_t t_pos = t.find("\v");
        key = t.substr(0, t_pos);
        cache[key] = file_pos;
        file_pos = file.tellg();
    }
}


OpResult SingleFileLogEngine::get(std::string key)
{   
    std::string s;
    std::string t;
    file.clear();
    if (cache.find(key) == cache.end()) {
        return OpResult {false, std::nullopt, "Key missing"};
    }
    file.seekg(cache.at(key), file.beg);
    std::getline( file, t );
    size_t pos = t.find("\v");
    if (pos+1 == t.length())
        return OpResult {false, std::nullopt, "Key missing"};
        
    return OpResult {true, t.substr(pos+1, t.length()), std::nullopt};
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
