#include "commands.hpp"
#include "engine.hpp"
#include "server.c"
#include "ui.hpp"
#include <iostream>

SimpleInMemoryEngine engine;

int main()
{

    // ConsoleUI ui { &engine };

    // ui.start_ui();

    callback = &(Parser::parse);
    start_listening();

    return 0;
}