#include <iostream>
#include <memory>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <fstream>

#include <fmt/format.h>

#include "append_log.hpp"


namespace fs = std::filesystem;


#define DATA_SUBDIR "aroww-db"

struct SegmentsComparator
{
  bool operator() (std::shared_ptr<SegmentFile> &lhs, std::shared_ptr<SegmentFile> &rhs)
  {
    return lhs->number < rhs->number;
  }
};


void AppendLogEngine::load_segment(std::shared_ptr<SegmentFile> segment) {
    std::fstream segment_file(segment->path, std::ios::in);

    std::string t, key;
    auto file_pos = segment_file.tellg();
    while (std::getline( segment_file, t )) {
        size_t t_pos = t.find("\v");
        key = t.substr(0, t_pos);
        cache[key] = std::make_pair(segment, file_pos);
        file_pos = segment_file.tellg();
    }
    segment->length = file_pos;
}

AppendLogEngine::AppendLogEngine(AppendLogConfiguration conf_): conf(conf_) {
    fs::path dir = conf.dir_path / DATA_SUBDIR;

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

        std::shared_ptr<SegmentFile> segment = std::make_shared<SegmentFile>(
            p.path(),
            std::stoi(match[1].str()),
            (match[2].str().length() != 0)
        );
        segments.push_back(segment);
    }

    std::sort(segments.begin(), segments.end(), SegmentsComparator());

    for (auto& seg: segments) {
        load_segment(seg);
    }
    
    if (segments.size() > 0) {    
        auto last =  segments.back();
        
        if (last->length > (conf.max_segment_size / 2)) {
            std::shared_ptr<SegmentFile> segment = std::make_shared<SegmentFile>(
                dir / fmt::format("db_{0}.txt", last->number + 1),
                last->number+1,
                false
            );
            segments.push_back(segment);
        }
    } else {
        std::shared_ptr<SegmentFile> segment = std::make_shared<SegmentFile>(
            dir / "db_1.txt",
            1,
            false
        );
        segments.push_back(segment);
    }


    current.open(segments.back()->path, std::ios::app | std::ios::in);
}


OpResult AppendLogEngine::get(std::string key)
{   
    std::string s;
    std::string t;
    if (cache.find(key) == cache.end()) {
        return OpResult {false, std::nullopt, "Key missing"};
    }
    auto pair = cache.at(key);
    if (pair.first->path == segments.back()->path ) {
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

OpResult AppendLogEngine::set(std::string key, std::string value)
{
    cache[key] = std::make_pair(segments.back(), current.tellp());
    current << key << "\v" << value << '\n';
    current.flush();
    return OpResult {true, std::nullopt, std::nullopt};
}

OpResult AppendLogEngine::drop(std::string key)
{
    cache[key] = std::make_pair(segments.back(), current.tellp());
    current << key << "\v" << std::endl;
    current.flush();
    return OpResult {true, std::nullopt, std::nullopt};
}
