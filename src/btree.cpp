#include "tinydb/btree.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <vector>

namespace tinydb {

namespace {

static uint16_t read16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0]) |
           static_cast<uint16_t>(p[1]) << 8;
}

static void write16(uint8_t* p, uint16_t v) {
    p[0] = static_cast<uint8_t>(v & 0xFF);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
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

static int64_t read64(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= uint64_t(p[i]) << (8 * i);
    return static_cast<int64_t>(v);
}

static void write64(uint8_t* p, int64_t v) {
    uint64_t u = static_cast<uint64_t>(v);
    for (int i = 0; i < 8; ++i) p[i] = static_cast<uint8_t>((u >> (8 * i)) & 0xFF);
}

struct LeafData {
    uint32_t next{0};
    std::vector<std::pair<int64_t, std::string_view>> cells;
};

static LeafData load_leaf(Page& page) {
    LeafData leaf;
    const uint8_t* d = page.data.data();
    uint16_t ncell = read16(d);
    leaf.next = read32(d + 2);
    leaf.cells.reserve(ncell);
    size_t off = 6;
    for (uint16_t i = 0; i < ncell; ++i) {
        int64_t key = read64(d + off);
        uint16_t len = read16(d + off + 8);
        leaf.cells.emplace_back(key,
            std::string_view(reinterpret_cast<const char*>(d + off + 10), len));
        off += 10 + len;
    }
    return leaf;
}

static void store_leaf(Page& page, const LeafData& leaf) {
    uint8_t* d = page.data.data();
    write16(d, static_cast<uint16_t>(leaf.cells.size()));
    write32(d + 2, leaf.next);
    size_t off = 6;
    for (auto& cell : leaf.cells) {
        write64(d + off, cell.first);
        uint16_t len = static_cast<uint16_t>(cell.second.size());
        write16(d + off + 8, len);
        std::memcpy(d + off + 10, cell.second.data(), len);
        off += 10 + len;
    }
    if (off < PAGE_SIZE) std::memset(d + off, 0, PAGE_SIZE - off);
}

static size_t leaf_size(const LeafData& leaf) {
    size_t n = 6;
    for (auto& cell : leaf.cells) n += 10 + cell.second.size();
    return n;
}

static int64_t last_key(Page& page, uint32_t& next) {
    const uint8_t* d = page.data.data();
    uint16_t ncell = read16(d);
    next = read32(d + 2);
    if (ncell == 0) return std::numeric_limits<int64_t>::min();
    size_t off = 6;
    for (uint16_t i = 0; i < ncell - 1; ++i) {
        uint16_t len = read16(d + off + 8);
        off += 10 + len;
    }
    return read64(d + off);
}

} // namespace

BTree::BTree(Pager& p) : pager_(p) {}

uint32_t BTree::create_table() {
    uint32_t pgno = pager_.alloc();
    Page& page = pager_.get(pgno);
    uint8_t* d = page.data.data();
    write16(d, 0);
    write32(d + 2, 0);
    pager_.mark_dirty(page);
    return pgno;
}

void BTree::insert(uint32_t root, Key k, std::string_view payload) {
    uint32_t pgno = root;
    while (true) {
        Page& page = pager_.get(pgno);
        uint32_t next_pg;
        int64_t last = last_key(page, next_pg);
        if (k.rowid > last && next_pg != 0) {
            pgno = next_pg;
            continue;
        }
        LeafData leaf = load_leaf(page);
        auto it = std::lower_bound(leaf.cells.begin(), leaf.cells.end(), k.rowid,
            [](const auto& a, int64_t key){ return a.first < key; });
        if (it != leaf.cells.end() && it->first == k.rowid) {
            it->second = payload;
        } else {
            leaf.cells.insert(it, {k.rowid, payload});
        }
        if (leaf_size(leaf) <= PAGE_SIZE) {
            store_leaf(page, leaf);
            pager_.mark_dirty(page);
        } else {
            LeafData newleaf;
            size_t sz = 0;
            size_t i = 0;
            for (; i < leaf.cells.size(); ++i) {
                size_t cell_sz = 10 + leaf.cells[i].second.size();
                if (sz + cell_sz > PAGE_SIZE / 2 && i > 0) break;
                sz += cell_sz;
            }
            newleaf.cells.assign(leaf.cells.begin() + i, leaf.cells.end());
            leaf.cells.erase(leaf.cells.begin() + i, leaf.cells.end());
            uint32_t new_pgno = pager_.alloc();
            newleaf.next = leaf.next;
            leaf.next = new_pgno;
            Page& new_page = pager_.get(new_pgno);
            store_leaf(page, leaf);
            pager_.mark_dirty(page);
            store_leaf(new_page, newleaf);
            pager_.mark_dirty(new_page);
        }
        break;
    }
}

Cursor BTree::open(uint32_t root) { return Cursor{root, root, 0}; }

bool BTree::seek(Cursor& c, int64_t key) {
    uint32_t pgno = c.root;
    while (true) {
        Page& page = pager_.get(pgno);
        const uint8_t* d = page.data.data();
        uint16_t ncell = read16(d);
        uint32_t next = read32(d + 2);
        size_t off = 6;
        for (uint16_t i = 0; i < ncell; ++i) {
            int64_t k = read64(d + off);
            if (k >= key) {
                c.pgno = pgno;
                c.idx = i;
                return (k == key);
            }
            uint16_t len = read16(d + off + 8);
            off += 10 + len;
        }
        if (next == 0) {
            c.pgno = pgno;
            c.idx = ncell;
            return false;
        }
        pgno = next;
    }
}

bool BTree::next(Cursor& c) {
    Page& page = pager_.get(c.pgno);
    const uint8_t* d = page.data.data();
    uint16_t ncell = read16(d);
    if (c.idx + 1 < ncell) { ++c.idx; return true; }
    uint32_t next = read32(d + 2);
    if (next == 0) return false;
    c.pgno = next;
    c.idx = 0;
    Page& np = pager_.get(c.pgno);
    return read16(np.data.data()) > 0;
}

std::string_view BTree::read_payload(const Cursor& c) {
    Page& page = pager_.get(c.pgno);
    const uint8_t* d = page.data.data();
    uint16_t ncell = read16(d);
    size_t off = 6;
    for (uint16_t i = 0; i < ncell; ++i) {
        uint16_t len = read16(d + off + 8);
        if (static_cast<int>(i) == c.idx) {
            tmp_.assign(reinterpret_cast<const char*>(d + off + 10), len);
            return tmp_;
        }
        off += 10 + len;
    }
    tmp_.clear();
    return tmp_;
}

int64_t BTree::key(const Cursor& c) {
    Page& page = pager_.get(c.pgno);
    const uint8_t* d = page.data.data();
    uint16_t ncell = read16(d);
    size_t off = 6;
    for (uint16_t i = 0; i < ncell; ++i) {
        int64_t k = read64(d + off);
        uint16_t len = read16(d + off + 8);
        if (static_cast<int>(i) == c.idx) return k;
        off += 10 + len;
    }
    return 0;
}

bool BTree::check(uint32_t root) {
    uint32_t pgno = root;
    int64_t prev = std::numeric_limits<int64_t>::min();
    while (pgno != 0) {
        Page& page = pager_.get(pgno);
        LeafData leaf = load_leaf(page);
        if (leaf_size(leaf) > PAGE_SIZE) return false;
        for (auto& cell : leaf.cells) {
            if (cell.first < prev) return false;
            prev = cell.first;
        }
        pgno = leaf.next;
    }
    return true;
}

} // namespace tinydb

