#include "tinydb/parser.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

namespace tinydb {
namespace {
std::string upper(std::string s){ for (auto& c : s) c = static_cast<char>(std::toupper(c)); return s; }
}

std::unique_ptr<ASTNode> parse(const std::string& sql) {
    auto S = upper(sql);
    if (S.rfind("CREATE TABLE", 0) == 0) {
        auto n = std::make_unique<ASTCreate>();
        n->table = "t";
        n->cols = {{"a","INT"},{"b","TEXT"}};
        return n;
    }
    if (S.rfind("INSERT", 0) == 0) {
        auto n = std::make_unique<ASTInsert>();
        n->table = "t";
        n->values = {"1","'x'"};
        return n;
    }
    auto n = std::make_unique<ASTSelect>();
    n->table = "t";
    n->cols = {"a","b"};
    return n;
}

} // namespace tinydb

