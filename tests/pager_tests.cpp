#include "tinydb/pager.hpp"
#include <cassert>
#include <memory>

struct DummyStorage : tinydb::IStorage {
    void read(uint64_t, void*, size_t) override {}
    void write(uint64_t, const void*, size_t) override {}
    void sync() override {}
};

int main() {
    auto st = std::make_unique<DummyStorage>();
    tinydb::Pager p(std::move(st));
    auto& pg = p.get(1);
    assert(pg.dirty == false);
    p.mark_dirty(pg);
    assert(pg.dirty == true);
    p.flush();
    return 0;
}

