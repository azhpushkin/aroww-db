#include "engine/engine.hpp"

class SimpleSocketServer {
private:
    int port;

    AbstractEngine& engine;

    static void* start_connection_thread(void*);
public:
    SimpleSocketServer(int, AbstractEngine&);
    void start_listening();

};