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

#include <fmt/format.h>

#include "engine.hpp"


namespace fs = std::filesystem;

#define DATA_SUBDIR "aroww-db"
#define DUMP_MEMTABLE_SIZE_THRESHOLD 3
#define MERGE_SEGMENTS_THRESHOLD 2


DBEngine::DBEngine(EngineConfiguration conf_): conf(conf_) {
    data_dir = conf.dir_path / DATA_SUBDIR;

    if (!fs::exists(data_dir)) {
        fs::create_directory(data_dir);
    } 
    
    for(auto& p: fs::directory_iterator(data_dir)) {
        auto segment = Segment::parse_path(p.path());
        if (segment.has_value()) {
            segments.push_back(segment.value());
        }
    }

    // Sort in descending order, just like reads will behave
    segments.sort([](SegmentPtr& l, SegmentPtr &r) { return l->timestamp > r->timestamp;});
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
    std::lock_guard<std::mutex> guard(write_file_mutex);
    current_memtable[key] = value;
    switch_if_needed();
    return std::make_unique<MsgUpdateOkResp>();
}

std::unique_ptr<Message> DBEngine::drop(std::string key)
{
    std::lock_guard<std::mutex> guard(write_file_mutex);
    current_memtable[key] = std::nullopt;
    switch_if_needed();
    return std::make_unique<MsgUpdateOkResp>();
}


void DBEngine::switch_if_needed() {
    if (current_memtable.size() < DUMP_MEMTABLE_SIZE_THRESHOLD) {
        return;
    }

    auto new_segment = Segment::dump_memtable(current_memtable, data_dir);
    segments.push_front(new_segment);
    
    current_memtable.clear();


    if (segments.size() < MERGE_SEGMENTS_THRESHOLD) {
        return;
    }

    std::vector<SegmentPtr> to_merge(segments.begin(), segments.end());
    auto merged_segment = Segment::merge(to_merge);
    for (auto seg: segments) {
        fs::remove(seg->file_path);
    } 
    
    segments.clear();
    segments.push_front(merged_segment);
}