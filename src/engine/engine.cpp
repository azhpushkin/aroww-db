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

#include "common/string_or_tomb.hpp"

#include "engine.hpp"
#include "segment.hpp"
#include "workers.hpp"
#include "memtable.hpp"



namespace fs = std::filesystem;

#define DATA_DIR(conf) (conf.dir_path / "aroww-db")


unsigned int EngineConfiguration::DEFAULT_INDEX_STEP = 4;
unsigned int EngineConfiguration::DEFAULT_MAX_MEMTABLE_SIZE = 1024;
unsigned int EngineConfiguration::DEFAULT_READ_WORKERS_AMOUNT = 3;


EngineConfiguration::EngineConfiguration(fs::path p_) : dir_path(p_) {
    index_step = DEFAULT_INDEX_STEP;
    max_memtable_size = DEFAULT_MAX_MEMTABLE_SIZE;
    read_workers_amount = DEFAULT_READ_WORKERS_AMOUNT;
}



DBEngine::DBEngine(EngineConfiguration conf_): conf(conf_) {

    if (!fs::exists(DATA_DIR(conf))) {
        fs::create_directory(DATA_DIR(conf));
    } 

    std::regex sstable_re(".*/[0-9]+.sstable");
    
    std::smatch match;
    read_queue = std::make_shared<ReadQueue>();
    
    
    for(auto& p: fs::directory_iterator(DATA_DIR(conf))) {
        std::string temp = p.path().string();
        if(std::regex_match(temp, match, sstable_re)) {
            segments.push_back(std::make_shared<Segment>(p.path()));
        }
    }

    // Sort in descending order, just like reads will behave
    segments.sort([](SegmentPtr& l, SegmentPtr &r) { return l->timestamp > r->timestamp;});

    current_memtable = std::make_unique<MemTable>(DATA_DIR(conf));

    for(unsigned int i = 0; i < conf.read_workers_amount; i++) {
        auto worker = std::make_shared<ReadWorker>(this, read_queue);
        worker->th = new std::thread(&ReadWorker::start, worker.get());
        read_workers.push_back(worker);
    }
}

DBEngine::~DBEngine() {
    for(auto worker: read_workers) {
        worker->send_close_signal();
    }
    for(auto worker: read_workers) {
        worker->await_closing();
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
    current_memtable->set_value(key, value);
    switch_if_needed();
    return std::make_unique<MessageSetResponse>();
}

std::unique_ptr<Message> DBEngine::drop(std::string key)
{
    current_memtable->set_value(key, tomb::create());
    switch_if_needed();
    return std::make_unique<MessageSetResponse>();
}


void DBEngine::switch_if_needed() {
    if (current_memtable->container.size() < conf.max_memtable_size) {
        return;
    }

    int64_t timestamp = static_cast<int64_t>(time(NULL));
    auto new_segment = Segment::dump_memtable(*current_memtable, DATA_DIR(conf), timestamp, conf.index_step);
    segments.push_front(new_segment);
    
    current_memtable->cleanup();
    current_memtable = std::make_unique<MemTable>(DATA_DIR(conf));
}