#include <thread>

#include "engine/interface.hpp"


class RunningConnection {
private:
    int socket;
    AbstractEngine& engine;
    std::thread th;

    void start();
public:
    RunningConnection(int, AbstractEngine&);
};

class SimpleSocketServer {
private:
    int port;

    AbstractEngine& engine;

    static void start_connection_thread(SimpleSocketServer* server, int* socket);
public:
    SimpleSocketServer(int, AbstractEngine&);
    int start_listening();

};