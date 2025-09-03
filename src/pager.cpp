#include "tinydb/pager.hpp"

namespace tinydb {

namespace {
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

Pager::Pager(std::unique_ptr<IStorage> s) : storage_(std::move(s)) {
    auto header = std::make_unique<Page>();
    header->no = HEADER_PGNO;
    header->data.fill(0);
    storage_->read(0, header->data.data(), PAGE_SIZE);
    next_pgno_ = read32(header->data.data());
    if (next_pgno_ < 2) next_pgno_ = 2;
    cache_[HEADER_PGNO] = std::move(header);
}

Page& Pager::get(uint32_t pgno) {
    auto it = cache_.find(pgno);
    if (it == cache_.end()) {
        auto page = std::make_unique<Page>();
        page->no = pgno;
        page->data.fill(0);
        storage_->read(static_cast<uint64_t>(pgno - 1) * PAGE_SIZE,
                       page->data.data(), PAGE_SIZE);
        it = cache_.emplace(pgno, std::move(page)).first;
    }
    if (pgno >= next_pgno_) {
        next_pgno_ = pgno + 1;
        Page& hdr = *cache_[HEADER_PGNO];
        write32(hdr.data.data(), next_pgno_);
        hdr.dirty = true;
    }
    return *it->second;
}

uint32_t Pager::alloc() {
    uint32_t pgno = next_pgno_++;
    auto page = std::make_unique<Page>();
    page->no = pgno;
    page->data.fill(0);
    cache_[pgno] = std::move(page);
    Page& hdr = get(HEADER_PGNO);
    write32(hdr.data.data(), next_pgno_);
    mark_dirty(hdr);
    return pgno;
}

void Pager::mark_dirty(Page& p) { p.dirty = true; }

void Pager::flush() {
    for (auto& kv : cache_) {
        Page& p = *kv.second;
        if (p.dirty) {
            storage_->write(static_cast<uint64_t>(p.no - 1) * PAGE_SIZE,
                             p.data.data(), PAGE_SIZE);
            p.dirty = false;
        }
    }
    storage_->sync();
}

} // namespace tinydb

