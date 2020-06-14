#include <queue>
#include <mutex>
#include <condition_variable>

#include "workers.hpp"
#include "common/messages.hpp"
#include "common/serialization.hpp"


ReadTask::ReadTask(std::string key_): key(key_), msg(nullptr), cv(), m() {}
ReadQueue::ReadQueue(): q(), m(), c() {}
ReadWorker::ReadWorker(DBEngine* engine_, std::shared_ptr<ReadQueue> q_): engine(engine_), read_queue(q_) {}

void ReadQueue::push(std::shared_ptr<ReadTask> task) {
    std::unique_lock<std::mutex> lock(m);
    q.push(task);
    c.notify_one();
}
std::shared_ptr<ReadTask> ReadQueue::pop() {
    std::unique_lock<std::mutex> lock(m);
    c.wait(lock, [this]{return !q.empty();});
    auto read_task = q.front();
    q.pop();
    return read_task; 
}


void ReadWorker::start(ReadWorker* worker) {
    while(true) {
        auto task = worker->read_queue->pop();

        {
            std::unique_lock<std::mutex> lock(task->m);
            string_or_tomb value = worker->memtable_and_segments_lookup(task->key);

            auto msg = new MessageGetResponse();
            msg->value = value;
            task->msg = std::unique_ptr<Message>(msg);
            task->cv.notify_all();
        }
    }
}

string_or_tomb ReadWorker::memtable_and_segments_lookup(std::string key) {
    auto resp = std::make_unique<MessageGetResponse>();
    if (engine->current_memtable.find(key) != engine->current_memtable.end()) {
        return engine->current_memtable.at(key);
    }

    for (auto segment: engine->segments) {
        auto lookup_res = segment->lookup(key);
        if (lookup_res.has_value()) {
            return lookup_res.value();
        }
    }
    tomb t{};
    return t;
}


