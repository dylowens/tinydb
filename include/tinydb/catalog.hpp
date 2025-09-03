#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace tinydb {

class Pager;
class BTree;

struct TableInfo {
    std::string name;
    uint32_t root{0};
};

class Catalog {
public:
    Catalog() = default;
    Catalog(Pager& pager, BTree& bt);
    bool create_table(const std::string& name, uint32_t root);
    uint32_t create_table(const std::string& name);
    const TableInfo* lookup(const std::string& name) const;
private:
    Pager* pager_{nullptr};
    BTree* btree_{nullptr};
    uint32_t schema_root_{0};
    int64_t next_rowid_{1};
    std::unordered_map<std::string, TableInfo> tables_;
};

} // namespace tinydb

