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

#include "engine.hpp"


namespace fs = std::filesystem;

#define DATA_SUBDIR "aroww-db"
#define DUMP_MEMTABLE_SIZE_THRESHOLD 2


DBEngine::DBEngine(EngineConfiguration conf_): conf(conf_) {
    data_dir = conf.dir_path / DATA_SUBDIR;

    if (!fs::exists(data_dir)) {
        fs::create_directory(data_dir);
    } 
    
    for(auto& p: fs::directory_iterator(data_dir)) {
        auto segment = Segment::parse_path(p.path());
        if (segment.has_value()) {
            segments.push_back(std::make_shared<Segment>(std::move(segment.value())));
        }
    }

    // Sort in descending order, just like reads will behave
    segments.sort([](SegmentPnt& l, SegmentPnt &r) { return l->timestamp > r->timestamp;});
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
        if (value.size() == 0)  {
            return std::make_unique<MsgGetMissingResp>();
        } else {
            auto resp = std::make_unique<MsgGetOkResp>();
            resp->val = value;
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
    if (current_memtable.size() <= DUMP_MEMTABLE_SIZE_THRESHOLD) {
        return;
    }

    auto new_segment = Segment::dump_memtable(current_memtable, data_dir);
    segments.push_front(new_segment);
    
    current_memtable.clear();




    // // Compress if needed
    // std::vector<std::shared_ptr<Segment>> to_compress;
    // for(auto s: segments) {
    //     if (!s->compressed) {
    //         to_compress.push_back(s);
    //     }
    // }
    // segments.push_back(std::make_shared<Segment>(segments.back()->number+1, false));
    // write_file.close();
    // if (to_compress.size() < COMPRESS_AMOUNT) // Threshold
    // {
    //     write_file.open(segments.back()->get_path(this), std::ios::app | std::ios::in);
    //     return;
    // }

    // auto compressed = std::make_shared<Segment>(to_compress.back()->number, true);
    // std::fstream compressed_df(compressed->get_path(this), std::ios::app);

    
    // std::map<std::string, std::string> values;
    // for(auto s: to_compress) {
    //     std::fstream segment_file(s->get_path(this), std::ios::in);

    //     std::string t, key, val;
    //     while (std::getline( segment_file, t )) {
    //         size_t t_pos = t.find("\v");
    //         key = t.substr(0, t_pos);
    //         val = t.substr(t_pos+1, t.length());
    //         values[key] = val;
    //     }
    //     segments.remove(s);
    //     segment_file.close();
    //     fs::remove(s->get_path(this));
    // }

    // for (auto pair: values) {
    //     auto pos = compressed_df.tellp();
    //     compressed_df << pair.first << "\v" << pair.second << std::endl;
    //     cache[pair.first] = std::make_pair(compressed, pos);
    // }

    // segments.insert((++segments.rbegin()).base(), compressed);
    // write_file.open(segments.back()->get_path(this), std::ios::app | std::ios::in);
}