#include <iostream>
#include <memory>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <fstream>
#include <mutex>

#include <fmt/format.h>

#include "append_log.hpp"


namespace fs = std::filesystem;


#define DATA_SUBDIR "aroww-db"
#define COMPRESS_AMOUNT 3

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


int AppendLogEngine::load_segment(std::shared_ptr<SegmentFile> segment) {
    std::fstream segment_file(segment->get_path(this), std::ios::in);

    std::string t, key;
    auto file_pos = segment_file.tellg();
    while (std::getline( segment_file, t )) {
        size_t t_pos = t.find("\v");
        key = t.substr(0, t_pos);
        cache[key] = std::make_pair(segment, file_pos);
        file_pos = segment_file.tellg();
    }
    return file_pos;
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

    int last_pos = 0;
    for (auto& seg: segments) {
        last_pos = load_segment(seg);
    }
    
    if (segments.size() > 0) {    
        if (last_pos >= conf.max_segment_size) {
            segments.push_back(std::make_shared<SegmentFile>(segments.back()->number+1, false));
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
    std::lock_guard<std::mutex> guard(write_file_mutex);
    cache[key] = std::make_pair(segments.back(), write_file.tellp());
    write_file << key << "\v" << value << std::endl;
    switch_to_new_segment(write_file.tellp());
    return OpResult {true, std::nullopt, std::nullopt};
}

OpResult AppendLogEngine::drop(std::string key)
{
    std::lock_guard<std::mutex> guard(write_file_mutex);
    cache[key] = std::make_pair(segments.back(), write_file.tellp());
    write_file << key << "\v" << std::endl;
    switch_to_new_segment( write_file.tellp());
    return OpResult {true, std::nullopt, std::nullopt};
}


void AppendLogEngine::switch_to_new_segment(int write_pos) {
    if (write_pos < conf.max_segment_size) {
        return;
    }

    // Compress if needed
    std::vector<std::shared_ptr<SegmentFile>> to_compress;
    for(auto s: segments) {
        if (!s->compressed) {
            to_compress.push_back(s);
        }
    }
    segments.push_back(std::make_shared<SegmentFile>(segments.back()->number+1, false));
    write_file.close();
    if (to_compress.size() < COMPRESS_AMOUNT) // Threshold
    {
        write_file.open(segments.back()->get_path(this), std::ios::app | std::ios::in);
        return;
    }

    auto compressed = std::make_shared<SegmentFile>(to_compress.back()->number, true);
    std::fstream compressed_df(compressed->get_path(this), std::ios::app);

    
    std::map<std::string, std::string> values;
    for(auto s: to_compress) {
        std::fstream segment_file(s->get_path(this), std::ios::in);

        std::string t, key, val;
        while (std::getline( segment_file, t )) {
            size_t t_pos = t.find("\v");
            key = t.substr(0, t_pos);
            val = t.substr(t_pos+1, t.length());
            values[key] = val;
        }
        segments.remove(s);
        segment_file.close();
        fs::remove(s->get_path(this));
    }

    for (auto pair: values) {
        auto pos = compressed_df.tellp();
        compressed_df << pair.first << "\v" << pair.second << std::endl;
        cache[pair.first] = std::make_pair(compressed, pos);
    }

    segments.insert((++segments.rbegin()).base(), compressed);
    write_file.open(segments.back()->get_path(this), std::ios::app | std::ios::in);
}