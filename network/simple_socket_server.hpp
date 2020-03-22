#include "engine/commands.hpp"
#include "engine/engine.hpp"
#include <memory>

class SimpleSocketServer {
private:
    int port;

    static AbstractEngine* _engine;
    static SimpleSocketServer* _instance;
    static void parse_and_process_message(char*, char*);

public:
    SimpleSocketServer(int);
    void connect_engine(AbstractEngine* engine);
    void start_listening();
};