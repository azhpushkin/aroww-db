#include <iostream>
#include "ui.hpp"
#include "engine.hpp"



int main() {
    SimpleInMemoryEngine engine;  // TODO: 2-step initialize?


    ConsoleUI ui{engine};

    return 0;
}