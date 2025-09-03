#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace tinydb {

struct TableInfo {
    std::string name;
    uint32_t root{0};
};

class Catalog {
public:
    bool create_table(const std::string& name, uint32_t root);
    const TableInfo* lookup(const std::string& name) const;
private:
    std::unordered_map<std::string, TableInfo> tables_;
};

} // namespace tinydb

