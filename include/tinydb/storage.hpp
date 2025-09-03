// Storage interface and file-backed implementation
#pragma once
#include <cstdint>
#include <cstdio>
#include <string>

namespace tinydb {

class IStorage {
public:
    virtual ~IStorage() = default;
    virtual void read(uint64_t off, void* buf, size_t n) = 0;
    virtual void write(uint64_t off, const void* buf, size_t n) = 0;
    virtual void sync() = 0;
};

class FileStorage : public IStorage {
public:
    explicit FileStorage(const std::string& path);
    ~FileStorage() override;
    void read(uint64_t off, void* buf, size_t n) override;
    void write(uint64_t off, const void* buf, size_t n) override;
    void sync() override;
private:
    std::string path_;
    std::FILE* f_{nullptr};
};

} // namespace tinydb

