#include "tinydb/varint.hpp"
#include <cassert>
#include <random>

int main() {
    // random round-trip property
    std::mt19937_64 rng(123);
    for (int i = 0; i < 1000; ++i) {
        uint64_t v = rng();
        auto enc = tinydb::encode_varint(v);
        auto [dec, used] = tinydb::decode_varint(enc.data(), enc.size());
        assert(used == enc.size());
        assert(dec == v);
    }
    // extremes
    for (uint64_t v : {0ull, (1ull<<63) - 1, ~0ull}) {
        auto enc = tinydb::encode_varint(v);
        auto [dec, used] = tinydb::decode_varint(enc.data(), enc.size());
        assert(used == enc.size());
        assert(dec == v);
    }
    return 0;
}

