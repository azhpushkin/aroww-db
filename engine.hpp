#pragma once

#include <iostream>

class AbstractEngine {
    public:
        virtual void pass_command(std::string) {}; 
};


class SimpleInMemoryEngine: public AbstractEngine {
    public:
        void pass_command(std::string); 
};
