#include <queue>
#include <mutex>
#include <condition_variable>

#include "workers.hpp"
#include "common/messages.hpp"
#include "common/string_or_tomb.hpp"
using namespace std::chrono_literals;


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
    auto received_task = c.wait_for(lock, 500ms, [this]{return !q.empty();});
    if (received_task) {
        auto read_task = q.front();
        q.pop();
        return read_task; 
    } else {
        return nullptr;
    }
    
}


void ReadWorker::start(ReadWorker* worker) {
    while(!worker->close_scheduled) {
        auto task = worker->read_queue->pop();
        if (task == nullptr) {
            continue;
        }

        {
            std::unique_lock<std::mutex> lock(task->m);
            string_or_tomb value = worker->memtable_and_segments_lookup(task->key);

            auto msg = new MessageGetResponse();
            msg->value = value;
            task->msg = std::unique_ptr<Message>(msg);
            task->cv.notify_all();
        }
    }
    worker->close_finished();
}

string_or_tomb ReadWorker::memtable_and_segments_lookup(std::string key) {
    auto resp = std::make_unique<MessageGetResponse>();
    if (engine->current_memtable->container.find(key) != engine->current_memtable->container.end()) {
        return engine->current_memtable->container.at(key);
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


