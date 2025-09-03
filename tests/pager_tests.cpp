#include "tinydb/pager.hpp"
#include "tinydb/storage.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>

int main() {
    const char* path = "pager_test.db";
    uint32_t pgno;
    {
        auto st = std::make_unique<tinydb::FileStorage>(path);
        tinydb::Pager pager(std::move(st));
        pgno = pager.alloc();
        auto& pg = pager.get(pgno);
        const char* msg = "hello pager";
        std::memcpy(pg.data.data(), msg, std::strlen(msg));
        pager.mark_dirty(pg);
        pager.flush();
    }
    {
        auto st = std::make_unique<tinydb::FileStorage>(path);
        tinydb::Pager pager(std::move(st));
        auto& pg = pager.get(pgno);
        char buf[12]{};
        std::memcpy(buf, pg.data.data(), sizeof(buf));
        assert(std::memcmp(buf, "hello pager", 12) == 0);
    }
    std::remove(path);
    return 0;
}
