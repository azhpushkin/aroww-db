#include <iostream>
#include <memory>
#include <optional>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <fstream>
#include <ctime>
#include <cstdint>

#include <fmt/format.h>

#include "engine.hpp"
#include "utils/serialization.hpp"


#define SSTABLE_PATH(dir, num) (dir / fmt::format("{}.sstable", num))

namespace fs = std::filesystem;
/* 
* ## Structure of segment file:
* 8 bytes -> timestamp
* 8 bytes -> total amount of keys
* 8 bytes -> amount of indexed keys
* 8 bytes -> start of index section (position)
* m bytes -> contents
* n bytes -> index 
*/


Segment::Segment(std::int64_t s, fs::path d): timestamp(s), dir(d) {
    std::fstream sstable_file(SSTABLE_PATH(dir, timestamp), std::ios::binary | std::ios::in);

    // Skip timestamp for now
    unpack_int64(sstable_file, timestamp);
    unpack_int64(sstable_file, keys_amount);
    unpack_int64(sstable_file, index_amount);
    unpack_int64(sstable_file, index_start);

    sstable_file.seekg(index_start, sstable_file.beg);
    while (sstable_file.peek() != EOF) {
        std::string key;
        int64_t pos;

        unpack_string(sstable_file, key);
        unpack_int64(sstable_file, pos);
        
        index[key] = pos;
    }
}


void Segment::clear() {
    fs::remove(SSTABLE_PATH(dir, timestamp));
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


std::shared_ptr<Segment> Segment::dump_memtable(MemTable& mtbl, fs::path dir) {
    int64_t timestamp = static_cast<int64_t>(time(NULL));
    int64_t total_size = static_cast<int64_t>(mtbl.size());
    int64_t index_size = static_cast<int64_t>(mtbl.size());  // same as total currently
    int64_t index_start_pos = 0;  // filled later

    std::fstream sstable(SSTABLE_PATH(dir, timestamp), std::ios::binary | std::ios::out | std::ios::ate);
    pack_int64(sstable, timestamp);
    pack_int64(sstable, total_size);
    pack_int64(sstable, index_size);
    pack_int64(sstable, index_start_pos);


    SegmentIndex index;

    // write contents, remember positions for index creation
    for (auto pair: mtbl) {
        auto pos = sstable.tellp();
        
        pack_string(sstable, pair.first);
        pack_string_or_tomb(sstable, pair.second);

        index[pair.first] = pos;
    }

    index_start_pos = sstable.tellp();
    for (auto pair: index) {
        pack_string(sstable, pair.first);
        pack_int64(sstable, pair.second);
    }

    sstable.seekp(sizeof(int64_t) * 3, sstable.beg);
    pack_int64(sstable, index_start_pos);  // write start of index
    sstable.close();

    return std::make_shared<Segment>(timestamp, dir);
}


std::optional<std::string> Segment::lookup(std::string key) {
    std::fstream sstable_file(SSTABLE_PATH(dir, timestamp), std::ios::binary | std::ios::in);
    if (index.find(key) == index.end()) {
        return std::nullopt;  // not present
    }
    int pos = index.at(key);

    sstable_file.seekg(pos, sstable_file.beg);

    std::string key_from_file;
    std::optional<std::string> value;
    unpack_string(sstable_file, key_from_file);
    unpack_string_or_tomb(sstable_file, value);
    return value;
}