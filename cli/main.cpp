#include "tinydb/parser.hpp"
#include "tinydb/codegen.hpp"
#include "tinydb/catalog.hpp"
#include "tinydb/vm.hpp"
#include "tinydb/storage.hpp"
#include "tinydb/pager.hpp"
#include "tinydb/btree.hpp"
#include <cctype>
#include <iostream>
#include <memory>
#include <string>

int main() {
    using namespace tinydb;
    std::unique_ptr<Pager> pager;
    std::unique_ptr<BTree> btree;
    std::unique_ptr<Catalog> catalog;
    VM vm;
    std::string line;
    std::cout << "tinydb CLI\n";
    while (std::cout << "tinydb> " && std::getline(std::cin, line)) {
        if (line.empty()) continue;
        if (line[0] == '.') {
            if (line.rfind(".open", 0) == 0) {
                std::string path = line.substr(5);
                while (!path.empty() && std::isspace(static_cast<unsigned char>(path.front()))) path.erase(path.begin());
                pager = std::make_unique<Pager>(std::make_unique<FileStorage>(path));
                btree = std::make_unique<BTree>(*pager);
                catalog = std::make_unique<Catalog>(*pager, *btree);
                vm.set_env(*btree, *catalog);
            } else if (line == ".schema") {
                if (!catalog) { std::cout << "no db\n"; continue; }
                for (auto& kv : catalog->tables()) {
                    std::cout << kv.second.name << "\n";
                }
            } else if (line == ".quit" || line == ".exit") {
                break;
            } else {
                std::cout << "unknown command\n";
            }
            continue;
        }
        if (!catalog) { std::cout << "no database open\n"; continue; }
        auto ast = parse(line);
        if (!ast) { std::cout << "parse error\n"; continue; }
        if (auto c = dynamic_cast<ASTCreate*>(ast.get())) {
            catalog->create_table(c->table);
            if (pager) pager->flush();
            std::cout << "ok\n";
            continue;
        }
        auto prog = codegen(*ast, *catalog);
        vm.run(prog);
        if (pager) pager->flush();
        if (dynamic_cast<ASTSelect*>(ast.get())) {
            for (auto& row : vm.results()) {
                for (size_t i = 0; i < row.size(); ++i) {
                    if (i) std::cout << '|';
                    if (row[i].tag == ColTag::INT) std::cout << row[i].i;
                    else std::cout << row[i].s;
                }
                std::cout << "\n";
            }
        }
    }
    if (pager) pager->flush();
    return 0;
}

