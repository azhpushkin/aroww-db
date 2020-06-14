#include <thread>
#include <mutex>


#include "engine/interface.hpp"
#include "common/messages.hpp"


class RunningConnection {
public:
    RunningConnection(int, AbstractEngine&);
    void close_conn();
private:
    int socket;
    AbstractEngine& engine;
    std::thread th;
    bool close_scheduled;

    void start();
};

class SimpleSocketServer {
public:
    SimpleSocketServer(int, AbstractEngine&);
    int start_listening();
    void close();
    std::mutex ready_mutex;
private:
    int port;
    bool close_scheduled;

    AbstractEngine& engine;
    
    static void start_connection_thread(SimpleSocketServer* server, int* socket);
};