#include "tinydb/btree.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <vector>

namespace tinydb {

namespace {

enum : uint8_t { LEAF = 1, INTERNAL = 2 };

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

struct InternalData {
    uint32_t child0{0};
    std::vector<std::pair<int64_t, uint32_t>> cells; // key, right child
};

static LeafData load_leaf(Page& page) {
    LeafData leaf;
    const uint8_t* d = page.data.data();
    uint16_t ncell = read16(d + 2);
    leaf.next = read32(d + 4);
    leaf.cells.reserve(ncell);
    size_t off = 8;
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
    d[0] = LEAF;
    write16(d + 2, static_cast<uint16_t>(leaf.cells.size()));
    write32(d + 4, leaf.next);
    size_t off = 8;
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
    size_t n = 8;
    for (auto& cell : leaf.cells) n += 10 + cell.second.size();
    return n;
}

static InternalData load_internal(Page& page) {
    InternalData in;
    const uint8_t* d = page.data.data();
    uint16_t ncell = read16(d + 2);
    in.child0 = read32(d + 4);
    in.cells.reserve(ncell);
    size_t off = 8;
    for (uint16_t i = 0; i < ncell; ++i) {
        int64_t key = read64(d + off);
        uint32_t child = read32(d + off + 8);
        in.cells.emplace_back(key, child);
        off += 12;
    }
    return in;
}

static void store_internal(Page& page, const InternalData& in) {
    uint8_t* d = page.data.data();
    d[0] = INTERNAL;
    write16(d + 2, static_cast<uint16_t>(in.cells.size()));
    write32(d + 4, in.child0);
    size_t off = 8;
    for (auto& cell : in.cells) {
        write64(d + off, cell.first);
        write32(d + off + 8, cell.second);
        off += 12;
    }
    if (off < PAGE_SIZE) std::memset(d + off, 0, PAGE_SIZE - off);
}

static size_t internal_size(const InternalData& in) {
    return 8 + in.cells.size() * 12;
}

struct InsertResult { bool split{false}; int64_t key{0}; uint32_t pgno{0}; };

static InsertResult insert_node(BTree& t, uint32_t pgno, bool is_root,
                               Key k, std::string_view payload) {
    Page& page = t.pager().get(pgno);
    uint8_t type = page.data[0];
    if (type == LEAF || type == 0) {
        LeafData leaf = load_leaf(page);
        auto it = std::lower_bound(leaf.cells.begin(), leaf.cells.end(), k.rowid,
            [](const auto& a, int64_t key){ return a.first < key; });
        if (it != leaf.cells.end() && it->first == k.rowid) it->second = payload;
        else leaf.cells.insert(it, {k.rowid, payload});
        if (leaf_size(leaf) <= PAGE_SIZE) {
            store_leaf(page, leaf); t.pager().mark_dirty(page); return {};
        }
        if (is_root) {
            size_t sz = 0, i = 0;
            for (; i < leaf.cells.size(); ++i) {
                size_t cell_sz = 10 + leaf.cells[i].second.size();
                if (sz + cell_sz > PAGE_SIZE / 2 && i > 0) break;
                sz += cell_sz;
            }
            LeafData left, right;
            left.cells.assign(leaf.cells.begin(), leaf.cells.begin() + i);
            right.cells.assign(leaf.cells.begin() + i, leaf.cells.end());
            uint32_t left_pg = t.pager().alloc();
            uint32_t right_pg = t.pager().alloc();
            left.next = right_pg;
            right.next = leaf.next;
            Page& lp = t.pager().get(left_pg);
            Page& rp = t.pager().get(right_pg);
            store_leaf(lp, left); t.pager().mark_dirty(lp);
            store_leaf(rp, right); t.pager().mark_dirty(rp);
            InternalData root;
            root.child0 = left_pg;
            root.cells.push_back({right.cells.front().first, right_pg});
            store_internal(page, root); t.pager().mark_dirty(page);
            return {};
        } else {
            size_t sz = 0, i = 0;
            for (; i < leaf.cells.size(); ++i) {
                size_t cell_sz = 10 + leaf.cells[i].second.size();
                if (sz + cell_sz > PAGE_SIZE / 2 && i > 0) break;
                sz += cell_sz;
            }
            LeafData right;
            right.cells.assign(leaf.cells.begin() + i, leaf.cells.end());
            leaf.cells.erase(leaf.cells.begin() + i, leaf.cells.end());
            right.next = leaf.next;
            uint32_t new_pgno = t.pager().alloc();
            leaf.next = new_pgno;
            Page& new_page = t.pager().get(new_pgno);
            store_leaf(page, leaf); t.pager().mark_dirty(page);
            store_leaf(new_page, right); t.pager().mark_dirty(new_page);
            return {true, right.cells.front().first, new_pgno};
        }
    } else { // INTERNAL
        InternalData in = load_internal(page);
        uint32_t child = in.child0;
        size_t pos = 0;
        while (pos < in.cells.size() && k.rowid >= in.cells[pos].first) {
            child = in.cells[pos].second;
            ++pos;
        }
        auto res = insert_node(t, child, false, k, payload);
        if (!res.split) return {};
        in.cells.insert(in.cells.begin() + pos, {res.key, res.pgno});
        if (internal_size(in) <= PAGE_SIZE) {
            store_internal(page, in); t.pager().mark_dirty(page); return {};
        }
        if (is_root) {
            size_t mid = in.cells.size() / 2;
            InternalData left, right;
            left.child0 = in.child0;
            left.cells.assign(in.cells.begin(), in.cells.begin() + mid);
            right.child0 = in.cells[mid].second;
            right.cells.assign(in.cells.begin() + mid + 1, in.cells.end());
            int64_t up_key = in.cells[mid].first;
            uint32_t left_pg = t.pager().alloc();
            uint32_t right_pg = t.pager().alloc();
            Page& lp = t.pager().get(left_pg);
            Page& rp = t.pager().get(right_pg);
            store_internal(lp, left); t.pager().mark_dirty(lp);
            store_internal(rp, right); t.pager().mark_dirty(rp);
            InternalData root;
            root.child0 = left_pg;
            root.cells.push_back({up_key, right_pg});
            store_internal(page, root); t.pager().mark_dirty(page);
            return {};
        } else {
            size_t mid = in.cells.size() / 2;
            InternalData right;
            int64_t up_key = in.cells[mid].first;
            right.child0 = in.cells[mid].second;
            right.cells.assign(in.cells.begin() + mid + 1, in.cells.end());
            in.cells.erase(in.cells.begin() + mid, in.cells.end());
            uint32_t new_pgno = t.pager().alloc();
            Page& np = t.pager().get(new_pgno);
            store_internal(page, in); t.pager().mark_dirty(page);
            store_internal(np, right); t.pager().mark_dirty(np);
            return {true, up_key, new_pgno};
        }
    }
}

static bool seek_leaf(BTree& t, uint32_t pgno, int64_t key, Cursor& c) {
    Page& page = t.pager().get(pgno);
    const uint8_t* d = page.data.data();
    uint16_t ncell = read16(d + 2);
    size_t off = 8;
    for (uint16_t i = 0; i < ncell; ++i) {
        int64_t k = read64(d + off);
        if (k >= key) {
            c.pgno = pgno; c.idx = i; return (k == key);
        }
        uint16_t len = read16(d + off + 8);
        off += 10 + len;
    }
    c.pgno = pgno; c.idx = ncell; return false;
}

static bool seek_node(BTree& t, uint32_t pgno, int64_t key, Cursor& c) {
    Page& page = t.pager().get(pgno);
    uint8_t type = page.data[0];
    if (type == LEAF || type == 0) return seek_leaf(t, pgno, key, c);
    InternalData in = load_internal(page);
    uint32_t child = in.child0;
    for (auto& cell : in.cells) {
        if (key < cell.first) break;
        child = cell.second;
    }
    return seek_node(t, child, key, c);
}

static bool check_node(BTree& t, uint32_t pgno, int64_t min, int64_t max, int64_t& last) {
    Page& page = t.pager().get(pgno);
    uint8_t type = page.data[0];
    if (type == LEAF || type == 0) {
        LeafData leaf = load_leaf(page);
        if (leaf_size(leaf) > PAGE_SIZE) return false;
        for (auto& cell : leaf.cells) {
            if (cell.first < min || cell.first > max) return false;
            if (cell.first < last) return false;
            last = cell.first;
        }
        if (leaf.next != 0) {
            Page& np = t.pager().get(leaf.next);
            if (np.data[0] != LEAF) return false;
            LeafData nl = load_leaf(np);
            if (!nl.cells.empty() && nl.cells.front().first < last) return false;
        }
        return true;
    } else {
        InternalData in = load_internal(page);
        if (internal_size(in) > PAGE_SIZE) return false;
        int64_t prev = min;
        uint32_t child = in.child0;
        for (auto& cell : in.cells) {
            if (cell.first < prev || cell.first > max) return false;
            if (!check_node(t, child, prev, cell.first, last)) return false;
            child = cell.second;
            prev = cell.first;
        }
        return check_node(t, child, prev, max, last);
    }
}

} // namespace

BTree::BTree(Pager& p) : pager_(p) {}

uint32_t BTree::create_table() {
    uint32_t pgno = pager_.alloc();
    Page& page = pager_.get(pgno);
    uint8_t* d = page.data.data();
    d[0] = LEAF;
    write16(d + 2, 0);
    write32(d + 4, 0);
    pager_.mark_dirty(page);
    return pgno;
}

void BTree::insert(uint32_t root, Key k, std::string_view payload) {
    insert_node(*this, root, true, k, payload);
}

Cursor BTree::open(uint32_t root) { return Cursor{root, root, 0}; }

bool BTree::seek(Cursor& c, int64_t key) {
    return seek_node(*this, c.root, key, c);
}

bool BTree::next(Cursor& c) {
    Page& page = pager_.get(c.pgno);
    const uint8_t* d = page.data.data();
    uint16_t ncell = read16(d + 2);
    if (c.idx + 1 < ncell) { ++c.idx; return true; }
    uint32_t next = read32(d + 4);
    if (next == 0) return false;
    c.pgno = next; c.idx = 0; Page& np = pager_.get(c.pgno); return read16(np.data.data() + 2) > 0;
}

std::string_view BTree::read_payload(const Cursor& c) {
    Page& page = pager_.get(c.pgno);
    const uint8_t* d = page.data.data();
    uint16_t ncell = read16(d + 2);
    size_t off = 8;
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
    uint16_t ncell = read16(d + 2);
    size_t off = 8;
    for (uint16_t i = 0; i < ncell; ++i) {
        int64_t k = read64(d + off);
        uint16_t len = read16(d + off + 8);
        if (static_cast<int>(i) == c.idx) return k;
        off += 10 + len;
    }
    return 0;
}

bool BTree::check(uint32_t root) {
    int64_t last = std::numeric_limits<int64_t>::min();
    return check_node(*this, root, std::numeric_limits<int64_t>::min(),
                      std::numeric_limits<int64_t>::max(), last);
}

} // namespace tinydb

