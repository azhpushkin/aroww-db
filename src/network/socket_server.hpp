#include <thread>

#include "engine/interface.hpp"
#include "network/messages.hpp"


class RunningConnection {
public:
    RunningConnection(int, AbstractEngine&);
private:
    int socket;
    AbstractEngine& engine;
    std::thread th;

    void start();
    std::unique_ptr<Message> process_message(std::unique_ptr<Message>);
    std::unique_ptr<Message> process_message(std::unique_ptr<MsgGetReq>);
    std::unique_ptr<Message> process_message(std::unique_ptr<MsgSetReq>);
    std::unique_ptr<Message> process_message(std::unique_ptr<MsgDropReq>);
    
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