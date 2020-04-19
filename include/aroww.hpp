#include <iostream>
#include <optional>


class ArowwResult {
public:
    bool success;
    std::optional<std::string> value = std::nullopt;
    std::optional<std::string> error_msg = std::nullopt;
};

class ArowwConnection {
public:
    ArowwConnection(std::string host_, std::string port_);
    int open_conn();
    int close_conn();

    ArowwResult get(std::string key);
    ArowwResult set(std::string key, std::string value);
    ArowwResult drop(std::string key);
};