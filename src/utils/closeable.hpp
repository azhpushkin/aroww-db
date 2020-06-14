#include <atomic>
#include <mutex>
#include <condition_variable>

class Closeable {
public:
    Closeable();
    void send_close_signal();
    void await_closing();
protected:
    std::atomic<bool> close_scheduled = false;

    void close_finished();
private: 
    mutable std::mutex m;
};