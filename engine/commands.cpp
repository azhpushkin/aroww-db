#include "commands.hpp"
#include "engine.hpp"
#include <sstream>

// Staticmethods of Command
Command* Command::deserialize(std::string input) {
    int res;
    char key[128];
    char value[128];
    char* c_input = (char *)input.c_str();

    for (char* p = c_input; *p; ++p)
        *p = tolower(*p);

    Command* command;
    if ((res = sscanf(c_input, "<get;%[^>]s>", key)) == 1) {
        command = new CommandGet { std::string(key) };
    } else if ((res = sscanf(c_input, "<drop;%[^>]s>", key)) == 1) {
        command = new CommandDrop { std::string(key) };
    } else if ((res = sscanf(c_input, "<set;%[^;];%[^>]>", key, value)) == 2) {
        command = new CommandSet { std::string(key), std::string(value) };
    } else {
        throw UnknownCommandException();
    }
    // TODO: remove new in favour of smart pointers
    return command;
}

// CommandGet methods
CommandGet::CommandGet(std::string key_)
    : key(key_)
{
    this->type = GET;
}

std::string CommandGet::execute(AbstractEngine& engine)
{
    return engine.get(this->key);
}

std::string CommandGet::serialize()
{
    std::stringstream ss;
    ss << "<get;" << this->key <<">";
    return ss.str();
}

// CommandSet methods
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

std::string CommandSet::serialize()
{
    std::stringstream ss;
    ss << "<set;" << this->key << ";" << this->value << ">";
    return ss.str();
}

// CommandDrop methods
CommandDrop::CommandDrop(std::string key_)
    : key(key_)
{
    this->type = DROP;
}

std::string CommandDrop::execute(AbstractEngine& engine)
{
    return engine.drop(this->key);
}

std::string CommandDrop::serialize()
{
    std::stringstream ss;
    ss << "<drop;" << this->key <<">";
    return ss.str();
}