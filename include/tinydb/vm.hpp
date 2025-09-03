#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include "tinydb/btree.hpp"
#include "tinydb/catalog.hpp"
#include "tinydb/record.hpp"

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
    VM() = default;
    VM(BTree& bt, Catalog& cat) : btree_(&bt), catalog_(&cat) {}
    void set_env(BTree& bt, Catalog& cat) { btree_ = &bt; catalog_ = &cat; }
    int run(const std::vector<Instr>& prog);
    const std::vector<std::vector<Value>>& results() const { return results_; }
private:
    BTree* btree_{nullptr};
    Catalog* catalog_{nullptr};
    std::vector<Cursor> cursors_;
    std::vector<Value> regs_;
    std::vector<std::vector<Value>> results_;
    size_t last_row_cols_{0};
    int64_t next_rowid_{1};
};

} // namespace tinydb

