#include "commands.hpp"
#include "engine.hpp"

CommandGet::CommandGet(std::string key_): key(key_) {
    this->type = GET;
}

void CommandGet::execute (AbstractEngine& engine) {
    engine.get(this->key);
}

CommandSet::CommandSet(std::string key_, std::string value_): key(key_), value(value_) {
    this->type = SET;
}

void CommandSet::execute (AbstractEngine& engine) {
    engine.set(this->key, this->value);
}

CommandDrop::CommandDrop(std::string key_): key(key_) {
    this->type = DROP;
}

void CommandDrop::execute (AbstractEngine& engine) {
    engine.drop(this->key);
}