#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <unistd.h>
#include "ar.cpp"


std::vector<std::string> get_keys;
std::vector<std::string> set_keys;
std::vector<std::string> set_values;
std::vector<std::string> drop_keys;


void read_data() {
    std::string s;
    
    std::ifstream get_keys_txt("set_keys.txt");
    while(std::getline(get_keys_txt, s, '\n')) 
        get_keys.push_back(s);
    get_keys_txt.close();

    std::ifstream set_keys_txt("set_keys.txt");
    while(std::getline(set_keys_txt, s, '\n')) 
        set_keys.push_back(s);
    set_keys_txt.close();

    std::ifstream set_values_txt("set_values.txt");
    while(std::getline(set_values_txt, s, '\n')) 
        set_values.push_back(s);
    set_values_txt.close();

    std::ifstream drop_keys_txt("set_keys.txt");
    while(std::getline(drop_keys_txt, s, '\n')) 
        drop_keys.push_back(s);
    drop_keys_txt.close();
}

void pr(std::string what, std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end) {
std::cout << what << " done in " 
		<< std::chrono::duration_cast<std::chrono::seconds>(end - start).count()
		<< " secs" << std::endl;
}

int main() {
    auto start = std::chrono::steady_clock::now();
    read_data();
    auto end = std::chrono::steady_clock::now();
    pr("READ", start, end);


    ArowwConn conn{};

    std::cout << "Doing SET" << std::endl;
    start = std::chrono::steady_clock::now();
    for (int i = 0; i < set_keys.size(); i++) conn.set(set_keys[i], set_values[i]);
    end = std::chrono::steady_clock::now();
    pr("SET", start, end);

    std::cout << "Doing GET" << get_keys.size() << std::endl;
    start = std::chrono::steady_clock::now();
    for (int i = 0; i < get_keys.size(); i++) {
        conn.get(get_keys[i]);
    }
    end = std::chrono::steady_clock::now();
    pr("GET", start, end);

    std::cout << "Doing DROP" << std::endl;
    start = std::chrono::steady_clock::now();
    for (int i = 0; i < drop_keys.size(); i++) conn.drop(drop_keys[i]);
    end = std::chrono::steady_clock::now();
    pr("DROP", start, end);

    return 0;
}
