#include <iostream>
#include "commands.hpp"
#include "engine.hpp"
#include "ui.hpp"
#include "server.c"

SimpleInMemoryEngine engine;

int main()
{

    // ConsoleUI ui { &engine };

    // ui.start_ui();

    callback = &(Parser::parse);
    start_listening();

    return 0;
}