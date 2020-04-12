#include "engine/engine.hpp"
#include "network/socket_server.hpp"
#include <memory>

int main()
{
    SimpleInMemoryEngine engine;

    SimpleSocketServer socket_server { 3490, engine };
    socket_server.start_listening();

    return 0;
}