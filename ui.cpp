#include "ui.hpp"
#include "engine.hpp"
#include <iostream>

ConsoleUI::ConsoleUI(AbstractEngine engine): engine(engine) {
    std::cout << "Created new ConsoleUI;" << std::endl;
}


ConsoleUI::~ConsoleUI() {
    std::cout << "ConsoleUI destroyed;" << std::endl;
}


void ConsoleUI::start_ui() {
    std::cout << "Arrow DB [v0.0]" << std::endl;
    std::cout << "Available commands: " << std::endl;
    std::cout << " * set <key> <value> " << std::endl;
    std::cout << " * get <key> " << std::endl;
    std::cout << " * delete <key> " << std::endl;
    std::cout << " * exit " << std::endl << std::endl;
}