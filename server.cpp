#include "engine/engine.hpp"
#include "network/simple_socket_server.hpp"
#include <memory>

int main()
{
    SimpleInMemoryEngine engine;

    SimpleSocketServer socket_server { 3490 };
    socket_server.connect_engine(&engine);
    socket_server.start_listening();

    return 0;
}