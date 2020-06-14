#pragma once

#include <cstdint>
#include <filesystem>
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

#include "common/messages.hpp"
#include "common/serialization.hpp"

#include "interface.hpp"
#include "segment.hpp"
#include "workers.hpp"
#include "memtable.hpp"


class EngineConfiguration {
public:
    fs::path dir_path;

    unsigned int index_step;  // Index one of X elements (i.e. step = 4, then index contains 25% of keys)
    unsigned int max_memtable_size;  //  max size of memtable, bytes
    unsigned int read_workers_amount;  // Amount of worker threads for GET
    // NOTE: there is always just one worker for writing (SET, DROP)

    EngineConfiguration(fs::path dir);

    static unsigned int DEFAULT_INDEX_STEP;
    static unsigned int DEFAULT_MAX_MEMTABLE_SIZE;
    static unsigned int DEFAULT_READ_WORKERS_AMOUNT;
    
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

    std::unique_ptr<MemTable> current_memtable;
    
    void switch_if_needed();
    friend class ReadWorker;
};

