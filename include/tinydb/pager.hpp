#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "tinydb/storage.hpp"

namespace tinydb {

// Avoid conflict with system/emscripten PAGE_SIZE macro (64 KiB)
#ifdef PAGE_SIZE
#undef PAGE_SIZE
#endif
constexpr uint32_t PAGE_SIZE = 4096;
constexpr uint32_t HEADER_PGNO = 1;

struct Page {
    uint32_t no{};
    std::array<uint8_t, PAGE_SIZE> data{};
    bool dirty{false};
};

class Pager {
public:
    explicit Pager(std::unique_ptr<IStorage> s);
    Page& get(uint32_t pgno);
    uint32_t alloc();
    void mark_dirty(Page&);
    void flush();
private:
    std::unique_ptr<IStorage> storage_;
    std::unordered_map<uint32_t, std::unique_ptr<Page>> cache_;
    uint32_t next_pgno_{2};
};

} // namespace tinydb

