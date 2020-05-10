#pragma once

#include <optional>
#include <memory>
#include "network/messages.hpp"


class AbstractEngine {
public:
    virtual std::unique_ptr<Message> get(std::string key) = 0;
    virtual std::unique_ptr<Message> set(std::string key, std::string value) = 0;
    virtual std::unique_ptr<Message> drop(std::string key) = 0;
};



