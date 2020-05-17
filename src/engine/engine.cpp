#include <iostream>
#include <memory>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <fstream>
#include <vector>
#include <mutex>
#include <ctime>

#include <fmt/format.h>

#include "engine.hpp"
#include "utils/serialization.hpp"


namespace fs = std::filesystem;

#define DATA_SUBDIR "aroww-db"
#define MERGE_SEGMENTS_THRESHOLD 2


unsigned int EngineConfiguration::DEFAULT_INDEX_STEP = 4;
unsigned int EngineConfiguration::DEFAULT_MAX_SEGMENT_SIZE = 1000;
unsigned int EngineConfiguration::DEFAULT_MERGE_SEGMENTS_THRESHOLD = 3;


EngineConfiguration::EngineConfiguration(fs::path p_) : dir_path(p_) {
    index_step = DEFAULT_INDEX_STEP;
    max_segment_size = DEFAULT_MAX_SEGMENT_SIZE;
    merge_segments_threshold = DEFAULT_MERGE_SEGMENTS_THRESHOLD;
}



void DBEngine::load_memtable(fs::path p) {
    std::fstream memtable_path(p, std::ios::binary | std::ios::in);

    while (memtable_path.peek() != EOF) {
        std::string key;
        std::optional<std::string> value;

        unpack_string(memtable_path, key);
        unpack_string_or_tomb(memtable_path, value);
        current_memtable[key] = value;
    }
}


DBEngine::DBEngine(EngineConfiguration conf_): conf(conf_) {
    data_dir = conf.dir_path / DATA_SUBDIR;

    if (!fs::exists(data_dir)) {
        fs::create_directory(data_dir);
    } 
    std::regex sstable_re(".*/[0-9]+.sstable");
    std::regex memtable_re(".*memtable.txt");
    
    std::smatch match;
    
    
    for(auto& p: fs::directory_iterator(data_dir)) {
        std::string temp = p.path().string();
        if(std::regex_match(temp, match, sstable_re)) {
            segments.push_back(std::make_shared<Segment>(p.path()));
        }
        else if (std::regex_match(temp, match, memtable_re)) {
            load_memtable(p.path());
        }
    }

    // Sort in descending order, just like reads will behave
    segments.sort([](SegmentPtr& l, SegmentPtr &r) { return l->timestamp > r->timestamp;});

    memtable_file = std::fstream(data_dir / "memtable.txt", std::ios::binary | std::ios::app);
}


std::unique_ptr<Message> DBEngine::get(std::string key)
{   
    if (current_memtable.find(key) != current_memtable.end()) {
        auto val = current_memtable.at(key);
        if (val.has_value()) {
            auto resp = std::make_unique<MsgGetOkResp>();
            resp->val = val.value();
            return resp;
        }
        else {
            return std::make_unique<MsgGetMissingResp>();
        }
    }

    for (auto s: segments) {
        auto res = s->lookup(key);
        if (!res.has_value()) {
            continue;
        }
        auto value = res.value();
        if (std::holds_alternative<std::nullptr_t>(value)) {
            return std::make_unique<MsgGetMissingResp>();
        }
        else {
            auto resp = std::make_unique<MsgGetOkResp>();
            resp->val = std::get<std::string>(value);
            return resp;
        }
    }
    return std::make_unique<MsgGetMissingResp>();
}


std::unique_ptr<Message> DBEngine::set(std::string key, std::string value)
{
    current_memtable[key] = value;
    pack_string(memtable_file, key);
    pack_string_or_tomb(memtable_file, value);
    memtable_file.flush();
    switch_if_needed();
    return std::make_unique<MsgUpdateOkResp>();
}

std::unique_ptr<Message> DBEngine::drop(std::string key)
{
    current_memtable[key] = std::nullopt;
    pack_string(memtable_file, key);
    pack_string_or_tomb(memtable_file, std::nullopt);
    memtable_file.flush();
    switch_if_needed();
    return std::make_unique<MsgUpdateOkResp>();
}


void DBEngine::switch_if_needed() {
    if (current_memtable.size() < conf.max_segment_size) {
        return;
    }

    int64_t timestamp = static_cast<int64_t>(time(NULL));
    auto new_segment = Segment::dump_memtable(current_memtable, data_dir, timestamp, conf.index_step);
    segments.push_front(new_segment);
    
    current_memtable.clear();
    memtable_file.close();
    fs::remove(data_dir / "memtable.txt");


    memtable_file = std::fstream(data_dir / "memtable.txt", std::ios::binary | std::ios::app);


    if (segments.size() < MERGE_SEGMENTS_THRESHOLD) {
        return;
    }

    std::vector<SegmentPtr> to_merge(segments.begin(), segments.end());
    auto merged_segment = Segment::merge(to_merge, conf.index_step);
    for (auto seg: segments) {
        fs::remove(seg->file_path);
    } 

    segments.clear();
    segments.push_front(merged_segment);
}