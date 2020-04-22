#include "base_connection.hpp"
#include "aroww.hpp"

class ArowwConn: public BaseConnection {
private:
    ArowwConnection* c;
public:
    ArowwConn() {
        c = new ArowwConnection("localhost", "3490");
        c->open_conn();
    }

    void get(std::string k) {
        c->get(k);
    }

    void set(std::string k, std::string v) {
        c->set(k, v);
    }

    void drop(std::string k) {
        c->drop(k);
    }
};
