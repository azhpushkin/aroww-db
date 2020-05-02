#include <iostream>
#include <memory>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <regex>

#include <fmt/format.h>

#include "engine.hpp"


// TODO: we can make this a struct, default one
#define SWITCH_AFTER 100 // small for test only

namespace fs = std::filesystem;


void SingleFileLogEngine::load_segment(SegmentFile& segment) {
    std::fstream segment_file(segment.path, std::ios::in);

    std::string t, key;
    auto file_pos = segment_file.tellg();
    while (std::getline( segment_file, t )) {
        size_t t_pos = t.find("\v");
        key = t.substr(0, t_pos);
        cache[key] = std::make_pair(&segment, file_pos);
        file_pos = segment_file.tellg();
    }
    segment.length = file_pos;
}

SingleFileLogEngine::SingleFileLogEngine(fs::path path) {
    fs::path dir = path / "aroww-db";

    if (!fs::exists(dir)) {
        fs::create_directory(dir);
    } 
    
    std::string segment_number;
    std::regex rx(".*db_([0-9]+)(_compressed)?.txt");
    for(auto& p: fs::directory_iterator(dir)) {
        std::smatch match;
        std::string temp = p.path().string();
        
        if(!std::regex_match(temp, match, rx)) {
            continue;
        }

        segments.emplace_back(
            p.path(),
            std::stoi(match[1].str()),
            (match[2].str().length() != 0)
        );
    }

    std::sort(
        segments.begin(), segments.end(),
        [](SegmentFile& a, SegmentFile& b) {return a.number < b.number;}
    );

    for (auto& seg: segments) {
        load_segment(seg);
    }
    
    if (segments.size() > 0) {    
        SegmentFile last =  segments.back();
        
        if (last.length > (SWITCH_AFTER / 2)) {
            segments.emplace_back(
                dir / fmt::format("db_{0}.txt", last.number + 1),
                last.number+1,
                false
            );
        }
    } else {
        segments.emplace_back(
            dir / "db_1.txt",
            1,
            false
        );
    }


    current.open(segments.back().path, std::ios::app | std::ios::in);
}


OpResult SingleFileLogEngine::get(std::string key)
{   
    std::string s;
    std::string t;
    if (cache.find(key) == cache.end()) {
        return OpResult {false, std::nullopt, "Key missing"};
    }
    auto pair = cache.at(key);
    if (pair.first->path == segments.back().path ) {
        current.seekg(pair.second, current.beg);
        std::getline( current, t );
    } else {
        std::fstream segment_file(pair.first->path, std::ios::in);
        segment_file.seekg(pair.second, segment_file.beg);
        std::getline( segment_file, t );
    }
    
    size_t pos = t.find("\v");
    if (pos+1 == t.length()) {
        return OpResult {false, std::nullopt, "Key missing"};
    }
    else {
        return OpResult {true, t.substr(pos+1, t.length()), std::nullopt};
    }
}

OpResult SingleFileLogEngine::set(std::string key, std::string value)
{
    cache[key] = std::make_pair(&segments.back(), current.tellp());
    current << key << "\v" << value << '\n';
    current.flush();
    return OpResult {true, std::nullopt, std::nullopt};
}

OpResult SingleFileLogEngine::drop(std::string key)
{
    cache[key] = std::make_pair(&segments.back(), current.tellp());
    current << key << "\v" << std::endl;
    current.flush();
    return OpResult {true, std::nullopt, std::nullopt};
}
