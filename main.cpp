#include <iostream>
#include "ui.hpp"
#include "engine.hpp"
#include "commands.hpp"



int main() {
    SimpleInMemoryEngine engine;  // TODO: 2-step initialize?


    ConsoleUI ui{&engine};
    
    ui.start_ui();

    return 0;
}