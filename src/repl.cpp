#include "tinydb/repl.hpp"

#include "tinydb/parser.hpp"
#include "tinydb/codegen.hpp"
#include "tinydb/catalog.hpp"
#include "tinydb/vm.hpp"
#include "tinydb/storage.hpp"
#include "tinydb/pager.hpp"
#include "tinydb/btree.hpp"
#include <cctype>
#include <memory>
#include <string>

using namespace tinydb;

extern "C" int tinydb_process_line(const std::string& line, std::string& out) {
    static std::unique_ptr<Pager> pager;
    static std::unique_ptr<BTree> btree;
    static std::unique_ptr<Catalog> catalog;
    static VM vm;

    if (line.empty()) return 0;
    if (line[0] == '.') {
        if (line.rfind(".open", 0) == 0) {
            std::string path = line.substr(5);
            while (!path.empty() && std::isspace(static_cast<unsigned char>(path.front()))) {
                path.erase(path.begin());
            }
            pager = std::make_unique<Pager>(std::make_unique<FileStorage>(path));
            btree = std::make_unique<BTree>(*pager);
            catalog = std::make_unique<Catalog>(*pager, *btree);
            vm.set_env(*btree, *catalog);
        } else if (line == ".schema") {
            if (!catalog) { out += "no db\n"; return 0; }
            for (auto& kv : catalog->tables()) {
                out += kv.second.name;
                out += '\n';
            }
        } else if (line == ".quit" || line == ".exit") {
            return 1;
        } else {
            out += "unknown command\n";
        }
        return 0;
    }
    if (!catalog) { out += "no database open\n"; return 0; }
    auto ast = parse(line);
    if (!ast) { out += "parse error\n"; return 0; }
    if (auto c = dynamic_cast<ASTCreate*>(ast.get())) {
        catalog->create_table(c->table);
        if (pager) pager->flush();
        out += "ok\n";
        return 0;
    }
    auto prog = codegen(*ast, *catalog);
    vm.run(prog);
    if (pager) pager->flush();
    if (dynamic_cast<ASTSelect*>(ast.get())) {
        for (auto& row : vm.results()) {
            for (size_t i = 0; i < row.size(); ++i) {
                if (i) out.push_back('|');
                if (row[i].tag == ColTag::INT) out += std::to_string(row[i].i);
                else out += row[i].s;
            }
            out.push_back('\n');
        }
    }
    return 0;
}
