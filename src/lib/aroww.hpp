#include <cstdbool>
#include <string>
#include <memory>

#include "common/messages.hpp"

class ArowwDB {
public:
    ArowwDB(std::string host, std::string port);
    ~ArowwDB();

    std::unique_ptr<Message> get(std::string key);
    std::unique_ptr<Message> set(std::string key, std::string value);
    std::unique_ptr<Message> drop(std::string key);

private:
    std::string host;
    std::string port;
    int sockfd;

    void open_socket();
    std::unique_ptr<Message> get_result();
};

