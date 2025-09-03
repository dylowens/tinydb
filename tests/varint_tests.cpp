#include "tinydb/varint.hpp"
#include <cassert>
#include <vector>

int main() {
    for (uint64_t v : {0ull, 1ull, 127ull, 128ull, 300ull, (1ull<<32), (1ull<<40)}) {
        auto enc = tinydb::encode_varint(v);
        auto [dec, used] = tinydb::decode_varint(enc.data(), enc.size());
        assert(used == enc.size());
        assert(dec == v);
    }
    return 0;
}

