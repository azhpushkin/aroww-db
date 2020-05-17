#include <thread>
#include <mutex>


#include "engine/interface.hpp"
#include "network/messages.hpp"


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
    std::unique_ptr<Message> process_message(std::unique_ptr<Message>);
    std::unique_ptr<Message> process_message(std::unique_ptr<MsgGetReq>);
    std::unique_ptr<Message> process_message(std::unique_ptr<MsgSetReq>);
    std::unique_ptr<Message> process_message(std::unique_ptr<MsgDropReq>);
    
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