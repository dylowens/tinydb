#include "tinydb/parser.hpp"
#include <cctype>
#include <memory>
#include <string>
#include <vector>

namespace tinydb {
namespace {
class Parser {
public:
    explicit Parser(const std::string& s) : sql(s), pos(0) {}
    void skip_ws() {
        while (pos < sql.size() && std::isspace(static_cast<unsigned char>(sql[pos]))) ++pos;
    }
    bool match_kw(const std::string& kw) {
        skip_ws();
        size_t len = kw.size();
        if (pos + len > sql.size()) return false;
        for (size_t i = 0; i < len; ++i) {
            if (std::toupper(static_cast<unsigned char>(sql[pos + i])) != kw[i]) return false;
        }
        pos += len;
        return true;
    }
    bool consume(char c) {
        skip_ws();
        if (pos < sql.size() && sql[pos] == c) { ++pos; return true; }
        return false;
    }
    std::string parse_ident() {
        skip_ws();
        size_t start = pos;
        while (pos < sql.size() && (std::isalnum(static_cast<unsigned char>(sql[pos])) || sql[pos]=='_')) ++pos;
        return sql.substr(start, pos-start);
    }
    std::string parse_string() {
        skip_ws();
        if (pos >= sql.size() || sql[pos] != '\'') return {};
        size_t start = ++pos;
        while (pos < sql.size() && sql[pos] != '\'') ++pos;
        std::string s = sql.substr(start, pos-start);
        if (pos < sql.size()) ++pos; // consume closing quote
        return "'" + s + "'"; // keep quotes as in AST
    }
    std::string parse_number() {
        skip_ws();
        size_t start = pos;
        while (pos < sql.size() && std::isdigit(static_cast<unsigned char>(sql[pos]))) ++pos;
        return sql.substr(start, pos-start);
    }
    bool eof() {
        skip_ws();
        return pos >= sql.size();
    }
    const std::string& sql;
    size_t pos;
};

} // namespace

std::unique_ptr<ASTNode> parse(const std::string& sql) {
    Parser p(sql);
    if (p.match_kw("CREATE") && p.match_kw("TABLE")) {
        auto n = std::make_unique<ASTCreate>();
        n->table = p.parse_ident();
        if (!p.consume('(')) return nullptr;
        while (true) {
            std::string col = p.parse_ident();
            if (col.empty()) return nullptr;
            std::string type = p.parse_ident();
            if (type.empty()) return nullptr;
            n->cols.emplace_back(col, type);
            if (p.consume(')')) break;
            if (!p.consume(',')) return nullptr;
        }
        if (!p.eof()) return nullptr;
        return n;
    }
    p.pos = 0; // reset
    if (p.match_kw("INSERT") && p.match_kw("INTO")) {
        auto n = std::make_unique<ASTInsert>();
        n->table = p.parse_ident();
        if (!p.match_kw("VALUES")) return nullptr;
        if (!p.consume('(')) return nullptr;
        while (true) {
            p.skip_ws();
            std::string val;
            if (p.pos < p.sql.size() && p.sql[p.pos] == '\'') val = p.parse_string();
            else val = p.parse_number();
            if (val.empty()) return nullptr;
            n->values.push_back(val);
            if (p.consume(')')) break;
            if (!p.consume(',')) return nullptr;
        }
        if (!p.eof()) return nullptr;
        return n;
    }
    p.pos = 0;
    if (p.match_kw("SELECT")) {
        auto n = std::make_unique<ASTSelect>();
        if (p.consume('*')) {
            n->cols.push_back("*");
        } else {
            std::string col = p.parse_ident();
            if (col.empty()) return nullptr;
            n->cols.push_back(col);
            while (p.consume(',')) {
                std::string c2 = p.parse_ident();
                if (c2.empty()) return nullptr;
                n->cols.push_back(c2);
            }
        }
        if (!p.match_kw("FROM")) return nullptr;
        n->table = p.parse_ident();
        if (n->table.empty()) return nullptr;
        if (p.match_kw("WHERE")) {
            if (!p.match_kw("ROWID")) return nullptr;
            if (!p.consume('=')) return nullptr;
            std::string num = p.parse_number();
            if (num.empty()) return nullptr;
            n->where_rowid = true;
            n->rowid = std::stoll(num);
        }
        if (!p.eof()) return nullptr;
        return n;
    }
    return nullptr;
}

} // namespace tinydb

