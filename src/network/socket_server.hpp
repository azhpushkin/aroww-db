#include "engine/engine.hpp"

class SimpleSocketServer {
private:
    int port;

    AbstractEngine& engine;

    static void start_connection_thread(SimpleSocketServer* server, int* socket);
public:
    SimpleSocketServer(int, AbstractEngine&);
    int start_listening();

};