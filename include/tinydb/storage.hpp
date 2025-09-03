#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include "tinydb/pager.hpp"

namespace tinydb {

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

