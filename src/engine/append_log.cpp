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

fs::path SegmentFile::get_path(AppendLogEngine* engine) {
    return engine->data_dir / fmt::format("db_{0}{1}.txt", number, compressed?"_compressed":"");
}

std::optional<SegmentFile> SegmentFile::parse_path(fs::path path) {
    std::regex rx(".*db_([0-9]+)(_compressed)?.txt");
    std::smatch match;
    std::string temp = path.string();
        
    if(!std::regex_match(temp, match, rx)) {
        return std::nullopt;
    }

    // Regex guarantees that match will work fine
    int number = std::stoi(match[1].str());
    bool compressed = (match[2].str().length() != 0);
    return SegmentFile{number, compressed};
}


void AppendLogEngine::load_segment(std::shared_ptr<SegmentFile> segment) {
    std::fstream segment_file(segment->get_path(this), std::ios::in);

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
    data_dir = conf.dir_path / DATA_SUBDIR;

    if (!fs::exists(data_dir)) {
        fs::create_directory(data_dir);
    } 
    
    for(auto& p: fs::directory_iterator(data_dir)) {
        auto segment = SegmentFile::parse_path(p.path());
        if (segment.has_value()) {
            segments.push_back(std::make_shared<SegmentFile>(segment.value()));
        }
    }

    segments.sort(SegmentsComparator());

    for (auto& seg: segments) {
        load_segment(seg);
    }
    
    if (segments.size() > 0) {    
        auto last =  segments.back();      
        if (last->length > (conf.max_segment_size / 2)) {
            segments.push_back(std::make_shared<SegmentFile>(last->number+1, false));
        }
    } else {
        segments.push_back(std::make_shared<SegmentFile>(1, false));
    }


    write_file.open(segments.back()->get_path(this), std::ios::app | std::ios::in);
}


OpResult AppendLogEngine::get(std::string key)
{   
    std::string s;
    std::string t;
    if (cache.find(key) == cache.end()) {
        return OpResult {false, std::nullopt, "Key missing"};
    }
    auto pair = cache.at(key);
    if (pair.first->number == segments.back()->number ) {
        write_file.seekg(pair.second, write_file.beg);
        std::getline( write_file, t );
    } else {
        
        std::fstream segment_file(pair.first->get_path(this), std::ios::in);
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
    cache[key] = std::make_pair(segments.back(), write_file.tellp());
    write_file << key << "\v" << value << std::endl;
    segments.back()->length = write_file.tellp();
    switch_to_new_segment();
    return OpResult {true, std::nullopt, std::nullopt};
}

OpResult AppendLogEngine::drop(std::string key)
{
    cache[key] = std::make_pair(segments.back(), write_file.tellp());
    write_file << key << "\v" << std::endl;
    segments.back()->length = write_file.tellp();
    switch_to_new_segment();
    return OpResult {true, std::nullopt, std::nullopt};
}


void AppendLogEngine::switch_to_new_segment() {
    if (segments.back()->length < conf.max_segment_size) {
        return;
    }

    write_file.close();
    segments.push_back(std::make_shared<SegmentFile>(segments.back()->number+1, false));
    write_file.open(segments.back()->get_path(this), std::ios::app | std::ios::in);
}