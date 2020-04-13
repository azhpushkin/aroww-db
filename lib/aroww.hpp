#include <iostream>
#include "proto_dist/commands.pb.h"


class ArowwResult {
public:
    bool success;
    std::optional<std::string> value = std::nullopt;
    std::optional<std::string> error_msg = std::nullopt;
};

class ArowwConnection {
private:
    std::string host;
    std::string port;
    
    int fd_socket;

    ArowwResult send_command(DBCommand&);

public:
    ArowwConnection(std::string host_, std::string port_) : host(host_), port(port_) {};
    int open_conn();
    int close_conn();

    ArowwResult get(std::string key);
    ArowwResult set(std::string key, std::string value);
    ArowwResult drop(std::string key);
};