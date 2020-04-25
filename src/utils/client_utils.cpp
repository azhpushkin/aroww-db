#include <sstream>
#include <fmt/format.h>
#include <regex>
#include <vector>
#include <tuple>
#include <map>

#include "client_utils.hpp"


std::vector<std::string> split_by_whitespaces(std::string&& src) {return split_by_whitespaces(src);}
std::vector<std::string> split_by_whitespaces(std::string& s) {
    if (!s.size()) return {};
    std::vector<std::string> vec;


    auto iss = std::istringstream{s};
    auto str = std::string{};

    while (iss >> str) vec.push_back(str);
    return vec;
}

Command parse_input(std::string&& input) {return parse_input(input);}
Command parse_input(std::string& input) {
    std::vector<std::string> args = split_by_whitespaces(input);
    
    
    Command com = {args.at(0), {}};
    args.erase(args.begin());  // remove first (command type) str from args
    com.args = args;

    // lower name of command
    std::transform (com.name.begin(), com.name.end(), com.name.begin(), ::tolower);

    return com;
}



Command get_command(std::ostream& out, std::istream& in) {
    std::string user_input;
    
    out << "cli> " << std::flush;
    if (!std::getline(in, user_input)) {
        return {"quit", {}};
    }

    // Trim start and end whitespaces
    user_input = std::regex_replace(user_input, std::regex("^ +| +$|( ) +"), "$1");
    if (user_input.length() == 0) {
        return {"skip", {}};
    }
    
    Command parsed = parse_input(user_input);

    // TODO: I guess this could be rewritten with macros
    std::map<std::string, size_t> known_commands = {
        {"get", 1},
        {"drop", 1},
        {"set", 2},
        {"quit", 0},
        {"help", 0}
    };
    auto res = known_commands.find(parsed.name);
    if (res == known_commands.end())
        return {"error", {fmt::format("Unknown command {}", parsed.name), }};

    if (res->second != parsed.args.size())
        return {
            "error",
            {fmt::format(
                "Expected {} arguments for {}, got {}",
                res->second,
                parsed.name,
                parsed.args.size()
            ), }
        };

    return parsed;
}