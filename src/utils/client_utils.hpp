#include <string>
#include <vector>

typedef struct {
    std::string name;
    std::vector<std::string> args;
} Command;

std::vector<std::string> split_by_whitespaces(std::string& src);
std::vector<std::string> split_by_whitespaces(std::string&& src);

Command parse_input(std::string& input);
Command parse_input(std::string&& input);

Command get_command(std::ostream& out, std::istream& in);
