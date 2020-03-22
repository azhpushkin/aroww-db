#include "commands.hpp"
#include "engine.hpp"
#include <stdio.h>
#include <string.h>

CommandGet::CommandGet(std::string key_)
    : key(key_)
{
    this->type = GET;
}

std::string CommandGet::execute(AbstractEngine& engine)
{
    return engine.get(this->key);
}

CommandSet::CommandSet(std::string key_, std::string value_)
    : key(key_)
    , value(value_)
{
    this->type = SET;
}

std::string CommandSet::execute(AbstractEngine& engine)
{
    return engine.set(this->key, this->value);
}

CommandDrop::CommandDrop(std::string key_)
    : key(key_)
{
    this->type = DROP;
}

std::string CommandDrop::execute(AbstractEngine& engine)
{
    return engine.drop(this->key);
}

void Parser::parse(char* buffer, char* result)
{
    int res;
    char key[128];
    char value[128];

    for (char* p = buffer; *p; ++p)
        *p = tolower(*p);

    if ((res = sscanf(buffer, "get %s", key)) == 1) {
        std::cout << "GET " << std::string(key) << std::endl;
    } else if ((res = sscanf(buffer, "drop %s", key)) == 1) {
        std::cout << "DROP " << std::string(key) << std::endl;
    } else if ((res = sscanf(buffer, "set %s %s", key, value)) == 2) {
        std::cout << "SET " << std::string(key) << " TO " << value << std::endl;
    }
    strcpy(result, "result\n");
}
