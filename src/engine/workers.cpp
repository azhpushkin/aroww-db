#include <queue>
#include <mutex>
#include <condition_variable>

#include "workers.hpp"


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
            auto res = worker->lookup(task->key);

            task->msg = std::move(res);
            task->cv.notify_all();
        }
    }
}

std::unique_ptr<Message> ReadWorker::lookup(std::string key) {
    if (engine->current_memtable.find(key) != engine->current_memtable.end()) {
        auto val = engine->current_memtable.at(key);
        if (val.has_value()) {
            auto resp = std::make_unique<MsgGetOkResp>();
            resp->val = val.value();
            return resp;
        }
        else {
            return std::make_unique<MsgGetMissingResp>();
        }
    }

    for (auto s: engine->segments) {
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


