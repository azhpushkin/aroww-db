#include "base_connection.cpp"
#include <pqxx/pqxx>



class PostgresConn: public BaseConnection {
private:
    pqxx::connection* c;
public:
    PostgresConn() {
        c = new pqxx::connection(
            "user=postgres "
            "host=localhost "
            "dbname=arowwbench");
    }

    void get(std::string k) {
        pqxx::work txn{*c};
        pqxx::result res = txn.exec("SELECT vvalue "
                                    "FROM kvstore "
                                    "WHERE kkey = " + txn.quote(k) + " LIMIT 1");
        txn.commit();
    }

    void set(std::string k, std::string v) {
        pqxx::work txn{*c};
        pqxx::result res = txn.exec("INSERT INTO kvstore "
                                    "VALUES (" + txn.quote(k) + ", " + txn.quote(v) + ") "
                                    "ON CONFLICT (kkey) DO UPDATE SET vvalue = excluded.vvalue");
        txn.commit();
    }

    void drop(std::string k) {
        pqxx::work txn{*c};
        pqxx::result res = txn.exec("DELETE FROM kvstore "
                                    "WHERE kkey = " + txn.quote(k));
        txn.commit();
    }
};
