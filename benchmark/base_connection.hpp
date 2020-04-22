#pragma once
#include <iostream>


class BaseConnection {
public:
    virtual void get(std::string) = 0;
    virtual void set(std::string, std::string) = 0;
    virtual void drop(std::string) = 0;
};
