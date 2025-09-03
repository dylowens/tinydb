#include "tinydb/varint.hpp"

namespace tinydb {

std::vector<uint8_t> encode_varint(uint64_t x) {
    std::vector<uint8_t> out;
    do {
        uint8_t byte = x & 0x7F;
        x >>= 7U;
        if (x) byte |= 0x80;
        out.push_back(byte);
    } while (x);
    return out;
}

std::pair<uint64_t,size_t> decode_varint(const uint8_t* p, size_t n) {
    uint64_t v = 0;
    size_t i = 0, shift = 0;
    while (i < n) {
        uint8_t b = p[i++];
        v |= static_cast<uint64_t>(b & 0x7F) << shift;
        if ((b & 0x80) == 0) break;
        shift += 7;
    }
    return {v, i};
}

} // namespace tinydb

