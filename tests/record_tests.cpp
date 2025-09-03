#include "tinydb/record.hpp"
#include <cassert>
#include <limits>
#include <random>

int main() {
    using tinydb::Value; using tinydb::ColTag;

    // random round-trip property
    std::mt19937_64 rng(123);
    std::uniform_int_distribution<int64_t> dist_i(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
    std::uniform_int_distribution<int> dist_len(0, 1000);
    for (int t = 0; t < 100; ++t) {
        std::vector<Value> row;
        row.push_back(Value{ColTag::INT, dist_i(rng), {}});
        std::string s(dist_len(rng), 'x');
        row.push_back(Value{ColTag::TEXT, 0, s});
        auto bytes = tinydb::encode_row(row);
        auto out = tinydb::decode_row(bytes.data(), bytes.size());
        assert(out.size() == row.size());
        assert(out[0].tag == ColTag::INT && out[0].i == row[0].i);
        assert(out[1].tag == ColTag::TEXT && out[1].s == row[1].s);
    }

    // extremes
    std::string big(100000, 'y');
    std::vector<Value> row2{
        {ColTag::INT, 0, {}},
        {ColTag::INT, -1, {}},
        {ColTag::INT, std::numeric_limits<int64_t>::max(), {}},
        {ColTag::TEXT, 0, big}
    };
    auto bytes2 = tinydb::encode_row(row2);
    auto out2 = tinydb::decode_row(bytes2.data(), bytes2.size());
    assert(out2.size() == row2.size());
    assert(out2[0].i == 0);
    assert(out2[1].i == -1);
    assert(out2[2].i == std::numeric_limits<int64_t>::max());
    assert(out2[3].s == big);
    return 0;
}

