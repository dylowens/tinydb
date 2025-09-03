#include "tinydb/catalog.hpp"

namespace tinydb {

bool Catalog::create_table(const std::string& name, uint32_t root) {
    return tables_.emplace(name, TableInfo{name, root}).second;
}

const TableInfo* Catalog::lookup(const std::string& name) const {
    auto it = tables_.find(name);
    if (it == tables_.end()) return nullptr;
    return &it->second;
}

} // namespace tinydb

