#include <filesystem>
#include <regex>

#include "common/serialization.hpp"
#include "common/string_or_tomb.hpp"
#include "memtable.hpp"

namespace fs = std::filesystem;

MemTable::MemTable(fs::path datadir) {
    std::regex memtable_re(".*memtable.txt");
    
    std::smatch match;
    for(auto& p: fs::directory_iterator(datadir)) {
        std::string temp = p.path().string();
        if(std::regex_match(temp, match, memtable_re)) {
            
            std::fstream memtable_path(p.path(), std::ios::binary | std::ios::in);

            while (memtable_path.peek() != EOF) {
                std::string key;
                string_or_tomb value;

                unpack_string(memtable_path, key);
                unpack_string_or_tomb(memtable_path, value);
                container[key] = value;
            }
            break;
        }
    }
    

    log_file = std::fstream(datadir / "memtable.txt", std::ios::binary | std::ios::app);
}


void MemTable::set_value(std::string key, string_or_tomb value) {
    container[key] = value;
    pack_string(log_file, key);
    pack_string_or_tomb(log_file, value);
    log_file.flush();
}