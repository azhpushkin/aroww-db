#pragma once

#include "engine.hpp"
#include <iostream>

// This command are only related to engine
// Commands like `exit`/`quit` are client-related, thus do not belong here
enum CommandType {
    GET,
    SET,
    DROP,
};

class Command {
public:
    virtual ~Command() {};
    CommandType type;
    virtual void execute(AbstractEngine&) = 0;
};

class CommandGet : public Command {
public:
    std::string key;
    CommandGet(std::string key);
    void execute(AbstractEngine&);
};

class CommandSet : public Command {
public:
    std::string key;
    std::string value;
    CommandSet(std::string key, std::string value);
    virtual void execute(AbstractEngine&);
};

class CommandDrop : public Command {
public:
    std::string key;
    CommandDrop(std::string key);
    void execute(AbstractEngine&);
};