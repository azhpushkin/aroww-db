#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <utility>
#include <list>
#include <mutex>
#include <optional>
#include <variant>
#include <vector>

#include "workers.fwd.hpp"
#include "engine.fwd.hpp"

#include "interface.hpp"
#include "segment.hpp"
#include "workers.hpp"

#include "common/messages.hpp"
#include "common/serialization.hpp"


class EngineConfiguration {
public:
    fs::path dir_path;
    unsigned int index_step;  // Index each X elements
    unsigned int max_segment_size;  //  Max amount of keys in single segment
    unsigned int merge_segments_threshold;  // Threshold before segments merge
    
    unsigned int read_workers;  // Amount of worker threads for GET
    // NOTE: there is always just one worker for writing (SET, DROP)

    EngineConfiguration(fs::path dir);

    static unsigned int DEFAULT_INDEX_STEP;
    static unsigned int DEFAULT_MAX_SEGMENT_SIZE;
    static unsigned int DEFAULT_MERGE_SEGMENTS_THRESHOLD;
    static unsigned int DEFAULT_READ_WORKERS;
    
};


class DBEngine : public AbstractEngine {
public:
    DBEngine(EngineConfiguration conf);
    std::unique_ptr<Message> get(std::string key);
    std::unique_ptr<Message> set(std::string key, std::string value);
    std::unique_ptr<Message> drop(std::string key);
private:
    EngineConfiguration conf;
    std::shared_ptr<ReadQueue> read_queue;
    std::vector<std::shared_ptr<ReadWorker>> read_workers;
    
    std::list<SegmentPtr> segments;

    MemTable current_memtable;
    std::fstream memtable_file;


    std::unique_ptr<Message> update_key(std::string key, string_or_tomb value);
    void switch_if_needed();
    friend class ReadWorker;
};

