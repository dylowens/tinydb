#include "tinydb/btree.hpp"
#include <algorithm>

namespace tinydb {

BTree::BTree(Pager& p) : pager_(p) {
    mem_.resize(1); // root 0 stub
}

uint32_t BTree::create_table() {
    mem_.push_back({});
    return static_cast<uint32_t>(mem_.size() - 1);
}

void BTree::insert(uint32_t root, Key k, std::string_view payload) {
    if (root >= mem_.size()) mem_.resize(root + 1);
    mem_[root].push_back({k.rowid, std::string(payload)});
    std::sort(mem_[root].begin(), mem_[root].end(),
              [](auto& a, auto& b){ return a.first < b.first; });
}

Cursor BTree::open(uint32_t root) {
    return Cursor{root, 0};
}

bool BTree::seek(Cursor& c, int64_t key) {
    auto& v = mem_[c.root];
    auto it = std::lower_bound(v.begin(), v.end(), key,
        [](auto& p, int64_t k){ return p.first < k; });
    c.idx = static_cast<int>(it - v.begin());
    return (it != v.end() && it->first == key);
}

bool BTree::next(Cursor& c) {
    auto& v = mem_[c.root];
    if (c.idx + 1 < static_cast<int>(v.size())) { ++c.idx; return true; }
    return false;
}

std::string_view BTree::read_payload(const Cursor& c) {
    auto& v = mem_[c.root];
    if (c.idx >= 0 && c.idx < static_cast<int>(v.size())) return v[c.idx].second;
    static std::string empty;
    return empty;
}

} // namespace tinydb

