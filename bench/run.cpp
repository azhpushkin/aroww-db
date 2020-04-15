#include <iostream>
#include "postgers_connection.cpp"


int main() {
    PostgresConn conn{};

    std::cout << "Doing SET" << std::endl;
    for (int i = 0; i < 10000; i++) {
        conn.set(std::to_string(i), std::to_string(i * 2));
    }

    std::cout << "Doing GET" << std::endl;
    for (int i = 0; i < 10000; i++) {
        conn.get(std::to_string(i*2));
    }

    std::cout << "Doing DROP" << std::endl;
    for (int i = 0; i < 10000; i++) {
        conn.drop(std::to_string(i));
    }

    return 0;
}
