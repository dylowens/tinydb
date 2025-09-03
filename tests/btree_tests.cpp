#include "tinydb/btree.hpp"
#include "tinydb/storage.hpp"
#include <algorithm>
#include <cassert>
#include <memory>
#include <random>
#include <string>
#include <vector>

int main() {
    const char* path = "btree_test.db";
    std::vector<int64_t> keys;
    uint32_t root;
    {
        auto st = std::make_unique<tinydb::FileStorage>(path);
        tinydb::Pager pager(std::move(st));
        tinydb::BTree t(pager);
        root = t.create_table();
        std::mt19937_64 rng(123);
        for (int i = 0; i < 1000; ++i) {
            int64_t k = static_cast<int64_t>(rng());
            keys.push_back(k);
            t.insert(root, {k}, "");
        }
        assert(t.check(root));
        std::sort(keys.begin(), keys.end());
        auto c = t.open(root);
        t.seek(c, keys.front());
        for (size_t i = 0; i < keys.size(); ++i) {
            assert(t.key(c) == keys[i]);
            if (i + 1 < keys.size()) assert(t.next(c));
            else assert(!t.next(c));
        }
        pager.flush();
    }

    std::sort(keys.begin(), keys.end());
    {
        auto st = std::make_unique<tinydb::FileStorage>(path);
        tinydb::Pager pager(std::move(st));
        tinydb::BTree t(pager);
        assert(t.check(root));
        auto c = t.open(root);
        t.seek(c, keys.front());
        for (size_t i = 0; i < keys.size(); ++i) {
            assert(t.key(c) == keys[i]);
            if (i + 1 < keys.size()) assert(t.next(c));
            else assert(!t.next(c));
        }
    }
    return 0;
}

