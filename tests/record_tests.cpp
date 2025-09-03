#include "tinydb/record.hpp"
#include <cassert>

int main() {
    using tinydb::Value; using tinydb::ColTag;
    std::vector<Value> row{{ColTag::INT, 42, {}}, {ColTag::TEXT, 0, "hello"}};
    auto bytes = tinydb::encode_row(row);
    auto out = tinydb::decode_row(bytes.data(), bytes.size());
    assert(out.size() == 2);
    assert(out[0].tag == ColTag::INT && out[0].i == 42);
    assert(out[1].tag == ColTag::TEXT && out[1].s == "hello");
    return 0;
}

