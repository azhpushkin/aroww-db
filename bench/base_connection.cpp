#include <iostream>


class BaseConnection {
public:
    void get(std::string);
    void set(std::string, std::string);
    void drop(std::string);
};
