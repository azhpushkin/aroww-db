#include <iostream>
#include <memory>
#include <optional>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <fstream>
#include <ctime>
#include <cstdint>
#include <variant>

#include <fmt/format.h>

#include "engine.hpp"
#include "utils/serialization.hpp"


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


std::shared_ptr<Segment> Segment::dump_memtable(MemTable& mtbl, fs::path dir, unsigned int index_step) {
    int64_t timestamp = static_cast<int64_t>(time(NULL));
    int64_t total_size = static_cast<int64_t>(mtbl.size());
    int64_t index_size = static_cast<int64_t>(mtbl.size());  // same as total currently
    int64_t index_start_pos = 0;  // filled later

    std::fstream sstable(SSTABLE_PATH(dir, timestamp), SSTABLE_WRITE_MODE);
    pack_int64(sstable, timestamp);
    pack_int64(sstable, total_size);
    pack_int64(sstable, index_size);
    pack_int64(sstable, index_start_pos);

    SegmentIndex index;

    auto index_i = index_step;
    for (auto pair: mtbl) {
        if (index_i == index_step) {
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

    sstable.seekp(sizeof(int64_t) * 3, sstable.beg);
    pack_int64(sstable, index_start_pos);  // write start of index
    sstable.close();

    return std::make_shared<Segment>(SSTABLE_PATH(dir, timestamp));
}


std::optional<std::variant<std::string, std::nullptr_t>> Segment::lookup(std::string key) {
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
        std::optional<std::string> value;
        unpack_string(sstable_file, key_from_file);
        unpack_string_or_tomb(sstable_file, value);
        
        if (key_from_file != key) {
            continue;
        }
        if (value.has_value()) {
            return value;
        } else {
            return nullptr;
        }
    }
    return std::nullopt;  // not found
    
}

SegmentPtr Segment::merge(std::vector<SegmentPtr> segments, unsigned int index_step) {
    // Prepare data of new segment
    auto latest = segments.front();
    std::fstream target_sstable(
        SSTABLE_PATH(latest->file_path.parent_path(), latest->timestamp + 1),
        SSTABLE_WRITE_MODE
    );
    int64_t timestamp = latest->timestamp + 1;
    int64_t total_size = 0;
    int64_t index_size = 0;
    int64_t index_start_pos = 0;
    pack_int64(target_sstable, timestamp);
    pack_int64(target_sstable, total_size);
    pack_int64(target_sstable, index_size);
    pack_int64(target_sstable, index_start_pos);
    SegmentIndex index;

    
    // Prepare old segments 
    int size = segments.size();
    int reached_end = 0;

    std::vector<int> statuses(size, 0);
    std::vector<std::string> loaded_keys(size, "");
    std::vector<std::fstream> files(size);
    for (int i = 0; i < size; i++) {
        files[i] = std::fstream(segments[i]->file_path, SSTABLE_READ_MODE);
        files[i].seekg(sizeof(int64_t) * 4);
    }


    auto index_i = index_step;
    // 0 - needs to read a string, 1 - string is loaded, 2 - reached end
    while (reached_end != size) {
        // Load keys from all files if needed
        for (int i = 0; i < size; i++) {
            if (statuses[i] == 0) {
                unpack_string(files[i], loaded_keys[i]);
                statuses[i] = 1;
            }
        }

        // TODO: skip elements with 2
        std::optional<std::string> min_iter = std::nullopt;
        for (int i = 0; i < size; i++) {
            if (statuses[i] == 2) continue;

            if (!min_iter.has_value() || loaded_keys[i] < min_iter.value()) {
                min_iter = loaded_keys[i];
            }
        }

        bool loaded = false;
        std::optional<std::string> val;

        for (int i = 0; i < size; i++) {
            if (loaded_keys[i] == min_iter) {
                unpack_string_or_tomb(files[i], val);
                if (!loaded) {
                    loaded = true;

                    if (index_i == index_step) {
                        auto pos = target_sstable.tellp();  // remember for index pos
                        index[min_iter.value()] = pos;
                        index_i = 0;
                    } else {
                        index_i++;
                    }
                    
                    pack_string(target_sstable, min_iter.value());
                    pack_string_or_tomb(target_sstable, val);
                    total_size++;
                    index_size++;
                }
                
                if (files[i].tellp() == segments[i]->index_start) {
                    reached_end++;
                    statuses[i] = 2;
                } else {
                    statuses[i] = 0;
                }
            }
        }
    }

    index_start_pos = target_sstable.tellp();
    for (auto pair: index) {
        pack_string(target_sstable, pair.first);
        pack_int64(target_sstable, pair.second);
    }

    target_sstable.seekp(target_sstable.beg);
    pack_int64(target_sstable, timestamp);
    pack_int64(target_sstable, total_size);
    pack_int64(target_sstable, index_size);
    pack_int64(target_sstable, index_start_pos);
    target_sstable.close();

    return std::make_shared<Segment>(SSTABLE_PATH(latest->file_path.parent_path(), timestamp));
}