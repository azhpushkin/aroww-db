#include <iostream>

#include <fmt/format.h>

#include "lib/aroww.hpp"
#include "utils/client_utils.hpp"






int main()
{
	ArowwConnection conn{std::string("localhost"), std::string("3490")};
	
    Command com;
	conn.open_conn();
    

    while(true) {
        ArowwResult res;
        com = get_command(std::cout, std::cin);
        
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
