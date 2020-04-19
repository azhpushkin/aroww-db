#include <fmt/format.h>
#include <regex>
#include <iostream>
#include <vector>
#include "lib/aroww.hpp"


class Command {
public:
    std::string name;
    std::vector<std::string> args;
};


std::vector<std::string> split(std::string s) {
    std::vector<std::string> vec;

    size_t pos = 0;
    std::string token;
    std::string delimeter = " ";
    while ((pos = s.find(delimeter)) != std::string::npos) {
        token = s.substr(0, pos);
        vec.push_back(s.substr(0, pos));
        s.erase(0, pos + delimeter.length());
    }
    vec.push_back(s.substr(0, pos));

    return vec;
}


Command parse_input(std::string input) {
    std::vector<std::string> args = split(input);
    
    Command com;
    com.name = args.at(0);
    std::transform (com.name.begin(), com.name.end(), com.name.begin(), ::tolower);

    args.erase(args.begin());
    com.args = args;

    return com;
}



int main()
{
	ArowwConnection conn{std::string("localhost"), std::string("3490")};
	ArowwResult res;

	conn.open_conn();
    std::string user_input;

    std::cout << "cli> " << std::flush;
    while(std::getline(std::cin, user_input)) {
        // Trim start and end whitespaces
        user_input = std::regex_replace(user_input, std::regex("^ +| +$|( ) +"), "$1");
        if (!user_input.length()) {
            std::cout << "cli> " << std::flush;
            continue;
        }

        Command com = parse_input(user_input);
        if (com.name == "get") {  // GET
            if (com.args.size() != 1) {
                std::cout << "Err: expected 1 argument for get, got "<< com.args.size() << std::endl;
                std::cout << "cli> " << std::flush;
                continue;
            }
            res = conn.get(com.args.at(0));
        } else if (com.name == "drop") {  // DROP
            if (com.args.size() != 1) {
                std::cout << "Err: expected 1 argument for drop, got "<< com.args.size() << std::endl;
                std::cout << "cli> " << std::flush;
                continue;
            }
            res = conn.drop(com.args.at(0));
        } else if (com.name == "set") {  // SET
            if (com.args.size() != 2) {
                std::cout << "Err: expected 2 argument for set, got "<< com.args.size() << std::endl;
                std::cout << "cli> " << std::flush;
                continue;
            }
            res = conn.set(com.args.at(0), com.args.at(1));
        } else if (com.name == "q") {
            break;
        } else {
            std::cout << "Command " << com.name << " is unknown!" << std::endl; 
            std::cout << "cli> " << std::flush;
            continue;
        }

        if (res.success) {
			std::cout << fmt::format("OK: {}", res.value.has_value() ? res.value.value() :"<empty>") << std::endl;
		} else {
			std::cout << fmt::format("ERR: {}", res.error_msg.value()) << std::endl;
		} 
        std::cout << "cli> " << std::flush;
    }


    std::cout << "Quitting!.." << std::endl;
}
