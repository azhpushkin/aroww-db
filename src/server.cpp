#include <iostream>
#include <filesystem>

#include "cxxopts.hpp"
#define SPDLOG_FMT_EXTERNAL 1
#include "spdlog/spdlog.h"

#include "engine/engine.hpp"
#include "network/socket_server.hpp"


int main(int argc, char* argv[])
{
    cxxopts::Options options("ArowwDB", "Simple key-value storage");
    options.add_options()
        ("v,verbose", "Output all info", cxxopts::value<bool>()->default_value("false"))
        ("d,datadir", "Path to directory for storing data", cxxopts::value<std::string>())
        ("i,index", "Index each N elements", cxxopts::value<unsigned int>()->default_value("5"))
        ("p,port", "Port for connections", cxxopts::value<int>()->default_value("7333"))
        ("h,help", "Show help message (you are reading it now)")
        ;

    auto result = options.parse(argc, argv);
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }
    if (result.count("verbose")) {
        spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    }
    else {
        spdlog::set_level(spdlog::level::info);
    }

    std::string path_input;
    try {
        path_input = result["datadir"].as<std::string>();
    }
    catch (std::domain_error &e) {
        // TODO: log error via some logger
        std::cerr << "Error: no path is given!" << std::endl;
        return 1;
    }

    std::filesystem::path path(path_input);
    std::error_code ec;
    if (!std::filesystem::is_directory(path, ec)) {
        std::cerr << "Error: path is not a directory" << std::endl;
        return 1;
    } 
    if (ec) {
        std::cerr << "Error with path: " << ec.message() << std::endl;
        return 1;
    }
    
    
    // small max size for test purposes
    EngineConfiguration conf{path};
    conf.index_step = result["index"].as<unsigned int>();
    DBEngine engine{conf};

    SimpleSocketServer socket_server { result["port"].as<int>(), engine };
    socket_server.start_listening();

    return 0;
}