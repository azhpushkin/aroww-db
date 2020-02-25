#include <iostream>
#include "engine.hpp"

void SimpleInMemoryEngine::pass_command(std::string command) {
    std::cout << "Engine received command: "<< command << std::endl;
}