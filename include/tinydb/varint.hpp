#pragma once
#include <cstdint>
#include <vector>
#include <utility>

namespace tinydb {

std::vector<uint8_t> encode_varint(uint64_t x);
std::pair<uint64_t, size_t> decode_varint(const uint8_t* p, size_t n);

} // namespace tinydb

