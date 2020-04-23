#include "engine/engine.hpp"
#include "network/socket_server.hpp"
#include <memory>

int main()
{
    SingleFileLogEngine engine{"/home/maqquettex/projects/aroww-db/build/file.txt"};

    SimpleSocketServer socket_server { 3490, engine };
    socket_server.start_listening();

    return 0;
}