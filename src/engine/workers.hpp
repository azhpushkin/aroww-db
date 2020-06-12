#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

#include "workers.fwd.hpp"
#include "engine.fwd.hpp"

#include "engine.hpp"
#include "network/messages.hpp"

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



class ReadWorker {
public:
    ReadWorker(DBEngine* engine, std::shared_ptr<ReadQueue> q);
    static void start(ReadWorker* worker);

private:
    DBEngine* engine;
    std::shared_ptr<ReadQueue> read_queue;

    std::unique_ptr<Message> lookup(std::string key);
};
