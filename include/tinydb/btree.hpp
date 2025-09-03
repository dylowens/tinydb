#pragma once
#include <cstdint>
#include <string_view>
#include <vector>
#include "tinydb/pager.hpp"

namespace tinydb {

struct Key { int64_t rowid{0}; };

class Cursor {
public:
    uint32_t root{0};
    int idx{0};
};

class BTree {
public:
    explicit BTree(Pager& p);
    uint32_t create_table();
    void insert(uint32_t root, Key k, std::string_view payload);
    Cursor open(uint32_t root);
    bool seek(Cursor& c, int64_t key);
    bool next(Cursor& c);
    std::string_view read_payload(const Cursor& c);
private:
    Pager& pager_;
    std::vector<std::vector<std::pair<int64_t, std::string>>> mem_; // stub
};

} // namespace tinydb

