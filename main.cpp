#include "commands.hpp"
#include "engine.hpp"
#include "ui.hpp"
#include <iostream>

int main()
{
    SimpleInMemoryEngine engine; // TODO: 2-step initialize?

    ConsoleUI ui { &engine };

    ui.start_ui();

    return 0;
}