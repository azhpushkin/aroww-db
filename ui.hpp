#pragma once

#include "engine.hpp"

class ConsoleUI {
private:
    AbstractEngine* engine;

public:
    ConsoleUI(AbstractEngine*);
    ~ConsoleUI();
    void start_ui();
};
