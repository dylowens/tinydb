#include "tinydb/record.hpp"
#include "tinydb/varint.hpp"

namespace tinydb {

std::vector<uint8_t> encode_row(const std::vector<Value>& cols) {
    std::vector<uint8_t> out;
    auto ncols = encode_varint(cols.size());
    out.insert(out.end(), ncols.begin(), ncols.end());
    for (auto& v : cols) {
        out.push_back(static_cast<uint8_t>(v.tag));
        if (v.tag == ColTag::INT) {
            auto enc = encode_varint(static_cast<uint64_t>(v.i));
            out.insert(out.end(), enc.begin(), enc.end());
        } else {
            auto len = encode_varint(v.s.size());
            out.insert(out.end(), len.begin(), len.end());
            out.insert(out.end(), v.s.begin(), v.s.end());
        }
    }
    return out;
}

std::vector<Value> decode_row(const uint8_t* p, size_t n) {
    std::vector<Value> cols;
    auto [cnt, used] = decode_varint(p, n);
    size_t off = used;
    for (size_t i = 0; i < cnt && off < n; ++i) {
        Value v;
        v.tag = static_cast<ColTag>(p[off++]);
        if (v.tag == ColTag::INT) {
            auto [ival, u2] = decode_varint(p + off, n - off);
            v.i = static_cast<int64_t>(ival);
            off += u2;
        } else {
            auto [len, u2] = decode_varint(p + off, n - off);
            off += u2;
            v.s.assign(reinterpret_cast<const char*>(p + off), static_cast<size_t>(len));
            off += static_cast<size_t>(len);
        }
        cols.push_back(std::move(v));
    }
    return cols;
}

} // namespace tinydb

