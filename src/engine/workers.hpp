#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <thread>

#include "workers.fwd.hpp"
#include "engine.fwd.hpp"

#include "engine.hpp"
#include "common/messages.hpp"
#include "common/serialization.hpp"
#include "utils/closeable.hpp"

class ReadTask {
public:
    std::string key;
    std::unique_ptr<Message> msg;
    std::condition_variable cv;
    std::mutex m;

    ReadTask(std::string key);
};

class ReadQueue {
public:
    ReadQueue();
    void push(std::shared_ptr<ReadTask>);
    std::shared_ptr<ReadTask> pop();
private:
  std::queue<std::shared_ptr<ReadTask>> q;
  mutable std::mutex m;
  std::condition_variable c;

};



class ReadWorker: public Closeable {
public:
    ReadWorker(DBEngine* engine, std::shared_ptr<ReadQueue> q);
    static void start(ReadWorker* worker);
    std::thread* th;
private:
    DBEngine* engine;
    std::shared_ptr<ReadQueue> read_queue;

    string_or_tomb memtable_and_segments_lookup(std::string key);
};
