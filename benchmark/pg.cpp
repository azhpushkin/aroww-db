#include "base_connection.hpp"
#include <libpq-fe.h>



class PostgresConn: public BaseConnection {
private:
    PGconn* c;
public:
    PostgresConn() {
        c = PQconnectdb("user=postgres host=127.0.0.1 dbname=arowwbench");
        if(PQstatus(c) != CONNECTION_OK)
            exit(1);
    }

    void get(std::string k) {
        const char* query =
        "SELECT vvalue "
        "FROM kvstore "
        "WHERE kkey = $1;";
        const char* params[1];
        params[0] = k.c_str();
        
        PQexecParams(c, query, 1, NULL, params, NULL, NULL, 0);
    }

    void set(std::string k, std::string v) {
        const char* query =
        "INSERT INTO kvstore (kkey, vvalue) "
        "VALUES ($1, $2) "
        "ON CONFLICT (kkey) DO UPDATE SET vvalue = excluded.vvalue;";
        const char* params[2];
        params[0] = k.c_str();
        params[1] = v.c_str();

        PQexecParams(c, query, 2, NULL, params, NULL, NULL, 0);
    }

    void drop(std::string k) {
        const char* query =
        "DELETE FROM kvstore "
        "WHERE kkey = $1;";
        const char* params[1];
        params[0] = k.c_str();

        PQexecParams(c, query, 1, NULL, params, NULL, NULL, 0);
            
    }
};
