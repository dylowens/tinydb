#pragma once
#include <array>
#include <cstdint>
#include <memory>

namespace tinydb {

struct Page {
    uint32_t no{};
    std::array<uint8_t, 4096> data{};
    bool dirty{false};
};

class IStorage {
public:
    virtual ~IStorage() = default;
    virtual void read(uint64_t off, void* buf, size_t n) = 0;
    virtual void write(uint64_t off, const void* buf, size_t n) = 0;
    virtual void sync() = 0;
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
    Page dummy_; // stub
};

} // namespace tinydb

