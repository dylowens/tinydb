#include "tinydb/catalog.hpp"
#include "tinydb/btree.hpp"
#include "tinydb/pager.hpp"
#include <cstring>
#include <limits>

namespace tinydb {

namespace {
static uint16_t read16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0]) |
           static_cast<uint16_t>(p[1]) << 8;
}
static uint32_t read32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) |
           static_cast<uint32_t>(p[1]) << 8 |
           static_cast<uint32_t>(p[2]) << 16 |
           static_cast<uint32_t>(p[3]) << 24;
}
static void write32(uint8_t* p, uint32_t v) {
    p[0] = static_cast<uint8_t>(v & 0xFF);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
    p[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
    p[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
}
} // namespace

Catalog::Catalog(Pager& pager, BTree& bt) : pager_(&pager), btree_(&bt) {
    Page& hdr = pager_->get(HEADER_PGNO);
    schema_root_ = read32(hdr.data.data() + 4);
    if (schema_root_ == 0) {
        schema_root_ = btree_->create_table();
        write32(hdr.data.data() + 4, schema_root_);
        pager_->mark_dirty(hdr);
    }
    Cursor c = btree_->open(schema_root_);
    btree_->seek(c, std::numeric_limits<int64_t>::min());
    Page& pg = pager_->get(c.pgno);
    if (read16(pg.data.data() + 2) == 0) { return; }
    while (true) {
        int64_t rowid = btree_->key(c);
        std::string_view payload = btree_->read_payload(c);
        if (payload.size() >= 4) {
            uint32_t root = read32(reinterpret_cast<const uint8_t*>(payload.data()));
            std::string name(payload.substr(4));
            tables_.emplace(name, TableInfo{name, root});
        }
        if (rowid >= next_rowid_) next_rowid_ = rowid + 1;
        if (!btree_->next(c)) break;
    }
}

bool Catalog::create_table(const std::string& name, uint32_t root) {
    if (!tables_.emplace(name, TableInfo{name, root}).second) return false;
    if (btree_ && schema_root_) {
        std::string payload;
        payload.resize(4 + name.size());
        write32(reinterpret_cast<uint8_t*>(payload.data()), root);
        std::memcpy(payload.data() + 4, name.data(), name.size());
        btree_->insert(schema_root_, {next_rowid_++}, payload);
    }
    return true;
}

uint32_t Catalog::create_table(const std::string& name) {
    if (!btree_) return 0;
    uint32_t root = btree_->create_table();
    if (!create_table(name, root)) return 0;
    return root;
}

const TableInfo* Catalog::lookup(const std::string& name) const {
    auto it = tables_.find(name);
    if (it == tables_.end()) return nullptr;
    return &it->second;
}

} // namespace tinydb

