#include <string>
#include <optional>

namespace aroww {
    class ArowwException : public std::exception {
    public:
        std::string message;
        ArowwException(std::string message_);
        const char * what () const noexcept;
    };


    class ArowwDB {
    public:
        std::string host;
        std::string port;
        int sockfd;

        ArowwDB(std::string host, std::string port);
        ~ArowwDB();

        std::optional<std::string> get(std::string key);
        void set(std::string key, std::string value);
        void drop(std::string key);
    };
}


