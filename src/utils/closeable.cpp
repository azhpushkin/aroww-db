#include <atomic>
#include <mutex>

#include "closeable.hpp"

Closeable::Closeable(): close_scheduled(false), m() {}

void Closeable::send_close_signal() {
    close_scheduled = true;
    m.try_lock();
}

void Closeable::await_closing() {
    std::unique_lock<std::mutex> lock(m);
}

void Closeable::close_finished() {
    m.unlock();
}
