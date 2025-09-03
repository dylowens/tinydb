#include "tinydb/storage.hpp"
#include <cstring>
#include <stdexcept>

namespace tinydb {

FileStorage::FileStorage(const std::string& path) : path_(path) {
    f_ = std::fopen(path.c_str(), "r+b");
    if (!f_) f_ = std::fopen(path.c_str(), "w+b");
    if (!f_) throw std::runtime_error("open failed");
}

FileStorage::~FileStorage() {
    if (f_) std::fclose(f_);
}

void FileStorage::read(uint64_t off, void* buf, size_t n) {
    std::memset(buf, 0, n);
    std::fseek(f_, static_cast<long>(off), SEEK_SET);
    std::fread(buf, 1, n, f_);
}

void FileStorage::write(uint64_t off, const void* buf, size_t n) {
    std::fseek(f_, static_cast<long>(off), SEEK_SET);
    std::fwrite(buf, 1, n, f_);
}

void FileStorage::sync() {
    std::fflush(f_);
}

} // namespace tinydb

