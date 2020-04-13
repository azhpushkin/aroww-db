#include <fmt/format.h>
#include "lib/aroww.hpp"


int main(int argc, char *argv[])
{
	ArowwConnection conn{std::string("localhost"), std::string("3490")};
	ArowwResult res;

	conn.open_conn();

	int sscanfres;
    char key[128];
    char value[128];
    char input[128];

    while(1) {

        std::cout << "cli> " << std::flush;
		fgets(input, 128, stdin);

        for (char* p = input; *p; ++p)
            *p = tolower(*p);

        if ((sscanfres = sscanf(input, "get %s", key)) == 1) {
			res = conn.get(std::string(key));
        } else if ((sscanfres = sscanf(input, "drop %s", key)) == 1) {
			res = conn.drop(std::string(key));
        } else if ((sscanfres = sscanf(input, "set %s %s", key, value)) == 2) {
			res = conn.set(std::string(key), std::string(value));
        } else if ((sscanfres = sscanf(input, "q")) == 1) {
			std::cout << "Quitting!.." << std::endl;
            break;
        } else {
			sscanf(input, "%s", input);  // truncate
			std::cout << "Command " << input << " is unknown" << std::endl;	
            continue;
        }

		if (res.success) {
			std::cout << fmt::format("OK: {}", res.value.has_value() ? res.value.value() :"<empty>") << std::endl;
		} else {
			std::cout << fmt::format("ERR: {}", res.error_msg.value()) << std::endl;
		}
        
    }

}
