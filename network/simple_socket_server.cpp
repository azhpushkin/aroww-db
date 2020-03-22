#include "simple_socket_server.hpp"
#include "engine/commands.hpp"
#include <iostream>
#include <string.h>

extern "C" {
void listen_on_port(int port, void (*callback)(char*, char*));
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

void SimpleSocketServer::parse_and_process_message(char* input, char* output)
{
    int res;
    char key[128];
    char value[128];

    for (char* p = input; *p; ++p)
        *p = tolower(*p);

    Command* command;

    if ((res = sscanf(input, "get %s", key)) == 1) {

        std::cout << "GET " << std::string(key) << std::endl;
        command = new CommandGet { std::string(key) };

    } else if ((res = sscanf(input, "drop %s", key)) == 1) {
        std::cout << "DROP " << std::string(key) << std::endl;
        command = new CommandDrop { std::string(key) };

    } else if ((res = sscanf(input, "set %s %s", key, value)) == 2) {
        std::cout << "SET " << std::string(key) << " TO " << value << std::endl;
        command = new CommandSet { std::string(key), std::string(value) };
    } else {
        std::cout << "Unknown command: " << std::string(input) << std::endl;
        sprintf(output, "Unknown command!\n");
        return;
    }

    std::string command_res = command->execute(*(SimpleSocketServer::_engine));
    sprintf(output, "%s\n", command_res.c_str());
}
