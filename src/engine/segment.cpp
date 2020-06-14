#include <iostream>
#include <memory>
#include <optional>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <variant>

#include <fmt/format.h>

#include "segment.hpp"
#include "common/serialization.hpp"


#define SSTABLE_PATH(dir, timestamp) (dir / fmt::format("{}.sstable", timestamp))
#define SSTABLE_WRITE_MODE std::ios::binary | std::ios::out | std::ios::ate
#define SSTABLE_READ_MODE std::ios::binary | std::ios::in

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


Segment::Segment(fs::path p): file_path(p) {
    std::fstream sstable_file(p, SSTABLE_READ_MODE);

    // Skip timestamp for now
    unpack_int64(sstable_file, timestamp);
    unpack_int64(sstable_file, keys_amount);
    unpack_int64(sstable_file, indexed_keys_amount);
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


std::shared_ptr<Segment> Segment::dump_memtable(MemTable& mtbl, fs::path dir, int64_t timestamp, unsigned int index_step) {
    int64_t total_size = static_cast<int64_t>(mtbl.container.size());
    int64_t index_size = static_cast<int64_t>(mtbl.container.size());  // same as total currently
    int64_t index_start_pos = 0;  // filled later

    std::fstream sstable(SSTABLE_PATH(dir, timestamp), SSTABLE_WRITE_MODE);
    pack_int64(sstable, timestamp);
    pack_int64(sstable, total_size);
    pack_int64(sstable, index_size);
    pack_int64(sstable, index_start_pos);

    SegmentIndex index;

    auto index_i = index_step - 1;
    for (auto pair: mtbl.container) {
        if (index_i == index_step - 1) {
            auto pos = sstable.tellp();  // remember for index posi
            index[pair.first] = pos;
            index_i = 0;
        } else {
            index_i++;
        }
        
        pack_string(sstable, pair.first);
        pack_string_or_tomb(sstable, pair.second);  
    }

    index_start_pos = sstable.tellp();
    for (auto pair: index) {
        pack_string(sstable, pair.first);
        pack_int64(sstable, pair.second);
    }

    index_size = index.size();
    sstable.seekp(sizeof(int64_t) * 2, sstable.beg);
    pack_int64(sstable, index_size);  // write index len
    pack_int64(sstable, index_start_pos);  // write start of index
    sstable.close();

    return std::make_shared<Segment>(SSTABLE_PATH(dir, timestamp));
}


std::optional<string_or_tomb> Segment::lookup(std::string key) {
    std::vector<std::string> keys;
    for (auto const& element : index) {
        keys.push_back(element.first);
    }

    auto pos_iter = std::upper_bound(keys.begin(), keys.end(), key);
    if (pos_iter == keys.begin()) {
        return std::nullopt;  // not present, first item greater than key
    }
    pos_iter--;  // finded element is first greater -> so get prev to find value

    std::fstream sstable_file(file_path, SSTABLE_READ_MODE);

    sstable_file.seekg(index[*pos_iter], sstable_file.beg);
    while (sstable_file.tellg() != index_start) {
        std::string key_from_file;
        string_or_tomb value;
        unpack_string(sstable_file, key_from_file);
        unpack_string_or_tomb(sstable_file, value);
        
        if (key_from_file == key) {
            return value;
        }
    }
    return std::nullopt;  // not found
    
}
