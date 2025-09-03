#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace tinydb {

enum class Op : uint8_t {
    OpenRead, OpenWrite, Rewind, SeekGE, Column,
    ResultRow, Next, Integer, Insert, Halt
};

struct Instr {
    Op op;
    int p1{0}, p2{0}, p3{0};
    std::string p4;
};

class VM {
public:
    int run(const std::vector<Instr>& prog);
};

} // namespace tinydb

