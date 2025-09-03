#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace tinydb {

enum class ColTag : uint8_t { INT = 0, TEXT = 1 };

struct Value {
    ColTag tag{ColTag::INT};
    int64_t i{0};
    std::string s{};
};

std::vector<uint8_t> encode_row(const std::vector<Value>& cols);
std::vector<Value>   decode_row(const uint8_t* p, size_t n);

} // namespace tinydb

