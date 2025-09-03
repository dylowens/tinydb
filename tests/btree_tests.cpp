#include "tinydb/btree.hpp"
#include <cassert>
#include <memory>

struct DummyStorage : tinydb::IStorage {
    void read(uint64_t, void*, size_t) override {}
    void write(uint64_t, const void*, size_t) override {}
    void sync() override {}
};

int main() {
    auto st = std::make_unique<DummyStorage>();
    tinydb::Pager pager(std::move(st));
    tinydb::BTree t(pager);
    auto root = t.create_table();
    t.insert(root, {2}, "b");
    t.insert(root, {1}, "a");
    auto c = t.open(root);
    bool found = t.seek(c, 1);
    assert(found);
    auto v = t.read_payload(c);
    assert(std::string(v) == "a");
    return 0;
}

