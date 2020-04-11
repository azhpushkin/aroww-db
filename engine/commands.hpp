#pragma once

#include "engine.hpp"
#include <iostream>
#include <exception>


// This command are only related to engine
// Commands like `exit`/`quit` are client-related, thus do not belong here
enum CommandType {
    GET,
    SET,
    DROP,
};

struct UnknownCommandException : public std::exception {
   const char * what () const throw () {
      return "Command uknown";
   }
};

class Command {
public:
    virtual ~Command() {};
    CommandType type;
    static Command* deserialize(std::string);
    virtual std::string execute(AbstractEngine&) = 0;
    virtual std::string serialize() = 0;
};

class CommandGet : public Command {
public:
    std::string key;
    CommandGet(std::string key);
    std::string execute(AbstractEngine&);
    std::string serialize();
};

class CommandSet : public Command {
public:
    std::string key;
    std::string value;
    CommandSet(std::string key, std::string value);
    virtual std::string execute(AbstractEngine&);
    std::string serialize();
};

class CommandDrop : public Command {
public:
    std::string key;
    CommandDrop(std::string key);
    std::string execute(AbstractEngine&);
    std::string serialize();
};
