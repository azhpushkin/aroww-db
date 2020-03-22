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