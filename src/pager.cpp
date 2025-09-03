#include "tinydb/pager.hpp"

namespace tinydb {

Pager::Pager(std::unique_ptr<IStorage> s) : storage_(std::move(s)) {}

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
    if (pgno >= next_pgno_) next_pgno_ = pgno + 1;
    return *it->second;
}

uint32_t Pager::alloc() {
    uint32_t pgno = next_pgno_++;
    auto page = std::make_unique<Page>();
    page->no = pgno;
    page->data.fill(0);
    cache_[pgno] = std::move(page);
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

