#include <iostream>
#include <memory>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <ctime>
#include <cstdint>

#include <fmt/format.h>

#include "engine.hpp"


#define INDEX_PATH(dir, num) (dir / fmt::format("{}.index", num))
#define SSTABLE_PATH(dir, num) (dir / fmt::format("{}.sstable", num))

namespace fs = std::filesystem;

// fs::path Segment::get_path(DBEngine* engine) {
//     return engine->data_dir / fmt::format("db_{0}{1}.txt", number, compressed?"_compressed":"");
// }


Segment::Segment(std::int64_t s, fs::path d): number(s), dir(d) {
    std::fstream index_file(INDEX_PATH(dir, number), std::ios::in);
    
    std::string t, key, val;
    while (std::getline( index_file, t )) {
        size_t t_pos = t.find("\v");
        key = t.substr(0, t_pos);
        val = t.substr(t_pos+1, t.length());
        index[key] = std::stoi(val);
    }
};
Segment::Segment(std::int64_t s, fs::path d, SegmentIndex&& i): number(s), dir(d), index(i) {};


void Segment::clear() {
    fs::remove(INDEX_PATH(dir, number));
    fs::remove(SSTABLE_PATH(dir, number));
}

std::optional<Segment> Segment::parse_path(fs::path path) {
    std::regex rx(".*/([0-9]+).sstable");
    std::smatch match;
    std::string temp = path.string();
        
    if(!std::regex_match(temp, match, rx)) {
        return std::nullopt;
    }
    std::int64_t number = std::stoi(match[1].str());
    if(!fs::exists(path.parent_path() / (match[1].str() + ".index"))) {
        // No matching index, skip
        return std::nullopt;
    }

    // Regex guarantees that match will work fine
    return Segment{number, path.parent_path()};
}


Segment Segment::dump_memtable(MemTable& mtbl, fs::path dir) {
    int64_t t = static_cast<int64_t>(time(NULL));
    std::fstream sstable(SSTABLE_PATH(dir, t), std::ios::app);
    std::fstream index_file(INDEX_PATH(dir, t), std::ios::app);

    SegmentIndex index;

    for (auto pair: mtbl) {
        auto pos = sstable.tellp();
        sstable << pair.first << "\v" << pair.second << std::endl;
        index_file << pair.first << "\v" << pos << std::endl;
        index[pair.first] = pos;
    }

    return Segment{t, dir, std::move(index)};
}


std::optional<std::string> Segment::lookup(std::string key) {
    if (index.find(key) == index.end()) {
        return std::nullopt;  // not present
    }
    int pos = index.at(key);
        
    std::fstream sstable(SSTABLE_PATH(dir, number), std::ios::in);
    sstable.seekg(pos, sstable.beg);

    std::string temp;
    std::getline(sstable, temp);
    size_t file_pos = temp.find("\v");
    if (file_pos+1 == temp.length()) {
        return "";  // TOMB, empty value means key removed
    }
    else {
        return temp.substr(file_pos+1, temp.length());
    }
}