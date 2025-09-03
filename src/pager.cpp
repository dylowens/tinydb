#include "tinydb/pager.hpp"

namespace tinydb {

Pager::Pager(std::unique_ptr<IStorage> s) : storage_(std::move(s)) {}

Page& Pager::get(uint32_t) {
    return dummy_;
}

uint32_t Pager::alloc() { return 1; }

void Pager::mark_dirty(Page& p) { p.dirty = true; }

void Pager::flush() {
    // stub
}

} // namespace tinydb

