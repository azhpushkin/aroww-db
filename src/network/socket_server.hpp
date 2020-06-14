#include <thread>
#include <mutex>


#include "engine/interface.hpp"
#include "common/messages.hpp"
#include "utils/closeable.hpp"


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

class SimpleSocketServer: public Closeable {
public:
    SimpleSocketServer(int, AbstractEngine&);
    int start_listening();
private:
    int port;
    int sockfd;

    AbstractEngine& engine;
    
    static void start_connection_thread(SimpleSocketServer* server, int* socket);
};