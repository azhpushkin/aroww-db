#include "engine.hpp"
#include <iostream>

EngineResult SimpleInMemoryEngine::get(std::string key)
{
    std::cout << "Engine received GET: " << key << std::endl;
    return OK;
}

EngineResult SimpleInMemoryEngine::set(std::string key, std::string value)
{
    std::cout << "Engine received SET: " << key << value << std::endl;
    return OK;
}

EngineResult SimpleInMemoryEngine::drop(std::string key)
{
    std::cout << "Engine received DROP: " << key << std::endl;
    return OK;
}
