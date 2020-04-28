#include <iostream>
#include <csignal>
#include <atomic>

#include <fmt/format.h>
#include <cxxopts.hpp>

#include "lib/aroww.hpp"
#include "utils/client_utils.hpp"

volatile std::atomic_bool EXIT_SCHEDULED = false;

void schedule_exit(int signal) {
    // TODO: better way of finishing this;
    if (signal == SIGINT)
        EXIT_SCHEDULED = true;
}

int main(int argc, char* argv[])
{
    cxxopts::Options options("ArowwDB", "Simple key-value storage");
    options.add_options()
        // TODO process verbose keyword to show different spdlog
        ("H,host", "Host", cxxopts::value<std::string>()->default_value("localhost"))
        ("p,port", "Port", cxxopts::value<std::string>()->default_value("7333"))
        ("h,help", "Show help message (you are reading it now)")
        ;

    auto result = options.parse(argc, argv);
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    std::signal(SIGINT, schedule_exit);

	ArowwConnection conn{
        result["host"].as<std::string>(),
        result["port"].as<std::string>()
    };
	
    Command com;
	conn.open_conn();
    

    while(true) {
        ArowwResult res;
        if (EXIT_SCHEDULED) {
            break;
        } else {
            com = get_command(std::cout, std::cin);
        }
        
        
        if (com.name == "get") res = conn.get(com.args.at(0));
        else if (com.name == "drop") res = conn.drop(com.args.at(0));
        else if (com.name == "set") res = conn.set(com.args.at(0), com.args.at(1));
        else if (com.name == "quit") break;
        else if (com.name == "help") {
            std::cout << "Hahaha no help yet, sorry!" << std::endl;
            continue;
        }
        else if (com.name == "error") {
            std::cout << "ERROR: " << com.args.at(0) << std::endl;
            continue;
        }
        else continue;  // assume "skip"

        if (res.success) {
			std::cout << fmt::format("OK: {}", res.value.has_value() ? res.value.value() :"<empty>") << std::endl;
		} else {
			std::cout << fmt::format("ERR: {}", res.error_msg.value()) << std::endl;
		} 
    }

    std::cout << "Quitting, bye!.." << std::endl;
}
