#include "simple_socket_server.hpp"
#include "engine/commands.hpp"
#include <iostream>
#include <string.h>

extern "C" {
void listen_on_port(int port, void (*callback)(char*, char*, int*));
}

SimpleSocketServer* SimpleSocketServer::_instance = nullptr;
AbstractEngine* SimpleSocketServer::_engine = nullptr;

SimpleSocketServer::SimpleSocketServer(int port_)
    : port(port_)
{
    // This is ugly I guess,
    // but this is the easiest way to get smth like global variable for now
    _instance = this;
}

void SimpleSocketServer::connect_engine(AbstractEngine* engine)
{
    SimpleSocketServer::_engine = engine;
}

void SimpleSocketServer::start_listening()
{
    listen_on_port(port, parse_and_process_message);
}

void SimpleSocketServer::parse_and_process_message(char* input, char* output, int* status)
{   
    Command* command;
    try {
        command = Command::deserialize(std::string(input));
    } catch (UnknownCommandException& e) {
        std::cout << "Unknown command " << input << " received!" << std::endl;
        *status = -1;
        return;
    }
    
    std::string command_res = command->execute(*(SimpleSocketServer::_engine));
    std::cout << "Executed command" << input << ", res is [" << command_res << "]" << std::endl;
    sprintf(output, "%s\n", command_res.c_str());
    if (command->type == GET) *status = 1;
    else                      *status = 0;
    delete command;
}
