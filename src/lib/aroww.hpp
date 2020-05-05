#include "commands.pb.h"

struct ArowwResult
{
    bool success;
    std::optional<std::string> value = std::nullopt;
    std::optional<std::string> error_msg = std::nullopt;
};

class ArowwConnection
{
private:
    std::string host;
    std::string port;

    int fd_socket;

    ArowwResult send_command(DBCommand &);

public:
    ArowwConnection(std::string host_, std::string port_);
    int open_conn();
    int close_conn();

    ArowwResult get(std::string key);
    ArowwResult set(std::string key, std::string value);
    ArowwResult drop(std::string key);
};


typedef struct {
    int is_ok;
    const char* value;
    const char* error_msg; 
} ConRes;

typedef struct  {
    void* cpp_conn;
} Connection;

extern "C" Connection* open_connection(char* host, char* port);
extern "C" void close_connection(Connection* conn);

extern "C" ConRes* connection_get(Connection* conn, char* key);
extern "C" ConRes* connection_set(Connection* conn, char* key, char* value);
extern "C" ConRes* connection_drop(Connection* conn, char* key);