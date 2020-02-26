#include "ui.hpp"
#include "engine.hpp"
#include "commands.hpp"
#include <iostream>
#include <memory>


ConsoleUI::ConsoleUI(AbstractEngine* engine): engine(engine) {
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
    std::cout << " * drop <key> " << std::endl;
    std::cout << " * exit " << std::endl << std::endl;

    std::string command, key, value;
    std::unique_ptr<Command> command_obj;

    while (true) {
        command_obj.reset();
        
        std::cout << "> " << std::flush;
        std::cin >> command;

        if (command == "exit")
        {
            break;
        }
        else if (command == "get")
        {
            std::cin >> key;
            command_obj = std::make_unique<CommandGet>(key);
        }
        else if (command == "set")
        {
            std::cin >> key;
            std::cin >> value;
            command_obj = std::make_unique<CommandSet>(key, value);
        }
        else if (command == "drop")
        {
            std::cin >> key;
            command_obj = std::make_unique<CommandDrop>(key);
        }
        else {
            std::cout << "Unknown operation: <" << command << ">!" << std::endl;
            continue;
        }

        command_obj->execute(*engine);
    }
}