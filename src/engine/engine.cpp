#include <iostream>
#include <memory>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <thread>
#include <fstream>
#include <vector>
#include <mutex>
#include <ctime>

#include "engine.hpp"
#include "segment.hpp"
#include "workers.hpp"
#include "common/serialization.hpp"


namespace fs = std::filesystem;

#define DATA_DIR(conf) (conf.dir_path / "aroww-db")
#define MERGE_SEGMENTS_THRESHOLD 2


unsigned int EngineConfiguration::DEFAULT_INDEX_STEP = 4;
unsigned int EngineConfiguration::DEFAULT_MAX_SEGMENT_SIZE = 1000;
unsigned int EngineConfiguration::DEFAULT_MERGE_SEGMENTS_THRESHOLD = 3;
unsigned int EngineConfiguration::DEFAULT_READ_WORKERS = 3;


EngineConfiguration::EngineConfiguration(fs::path p_) : dir_path(p_) {
    index_step = DEFAULT_INDEX_STEP;
    max_segment_size = DEFAULT_MAX_SEGMENT_SIZE;
    merge_segments_threshold = DEFAULT_MERGE_SEGMENTS_THRESHOLD;
    read_workers = DEFAULT_READ_WORKERS;
}



DBEngine::DBEngine(EngineConfiguration conf_): conf(conf_) {

    if (!fs::exists(DATA_DIR(conf))) {
        fs::create_directory(DATA_DIR(conf));
    } 
    std::regex sstable_re(".*/[0-9]+.sstable");
    std::regex memtable_re(".*memtable.txt");
    
    std::smatch match;
    read_queue = std::make_shared<ReadQueue>();
    
    
    for(auto& p: fs::directory_iterator(DATA_DIR(conf))) {
        std::string temp = p.path().string();
        if(std::regex_match(temp, match, sstable_re)) {
            segments.push_back(std::make_shared<Segment>(p.path()));
        }
        else if (std::regex_match(temp, match, memtable_re)) {
            current_memtable = load_memtable(p.path());
        }
    }

    // Sort in descending order, just like reads will behave
    segments.sort([](SegmentPtr& l, SegmentPtr &r) { return l->timestamp > r->timestamp;});

    memtable_file = std::fstream(DATA_DIR(conf) / "memtable.txt", std::ios::binary | std::ios::app);

    for(unsigned int i = 0; i < conf.read_workers; i++) {
        auto worker = std::make_shared<ReadWorker>(this, read_queue);
        read_workers.push_back(worker);
        new std::thread(&ReadWorker::start, worker.get());
    }

}


std::unique_ptr<Message> DBEngine::get(std::string key)
{  
    auto task = std::make_shared<ReadTask>(key);
    std::unique_lock<std::mutex> lock(task->m);
    read_queue->push(task);
    task->cv.wait(lock);
    return std::move(task->msg);
}


std::unique_ptr<Message> DBEngine::set(std::string key, std::string value)
{
    return update_key(key, value);
}

std::unique_ptr<Message> DBEngine::drop(std::string key)
{
    tomb t{};
    return update_key(key, t);
}

std::unique_ptr<Message> DBEngine::update_key(std::string key, string_or_tomb value) {
    current_memtable[key] = value;
    pack_string(memtable_file, key);
    pack_string_or_tomb(memtable_file, value);
    memtable_file.flush();
    switch_if_needed();
    return std::make_unique<MessageSetResponse>();
}


void DBEngine::switch_if_needed() {
    if (current_memtable.size() < conf.max_segment_size) {
        return;
    }

    int64_t timestamp = static_cast<int64_t>(time(NULL));
    auto new_segment = Segment::dump_memtable(current_memtable, DATA_DIR(conf), timestamp, conf.index_step);
    segments.push_front(new_segment);
    
    current_memtable.clear();
    memtable_file.close();
    fs::remove(DATA_DIR(conf) / "memtable.txt");


    memtable_file = std::fstream(DATA_DIR(conf) / "memtable.txt", std::ios::binary | std::ios::app);


    // if (segments.size() < MERGE_SEGMENTS_THRESHOLD) {
    //     return;
    // }

    // std::vector<SegmentPtr> to_merge(segments.begin(), segments.end());
    // auto merged_segment = Segment::merge(to_merge, conf.index_step);
    // for (auto seg: segments) {
    //     fs::remove(seg->file_path);
    // } 

    // segments.clear();
    // segments.push_front(merged_segment);
}