#include "tinydb/parser.hpp"
#include "tinydb/codegen.hpp"
#include "tinydb/catalog.hpp"
#include "tinydb/storage.hpp"
#include "tinydb/pager.hpp"
#include "tinydb/btree.hpp"
#include <cassert>
#include <memory>

int main() {
    using namespace tinydb;
    Pager pager(std::make_unique<FileStorage>("codegen.db"));
    BTree bt(pager);
    Catalog cat(pager, bt);
    cat.create_table("t");
    auto ast = parse("SELECT * FROM t");
    auto prog = codegen(*ast, cat);
    assert(!prog.empty());
    auto ast2 = parse("INSERT INTO t VALUES(1,'x')");
    auto prog2 = codegen(*ast2, cat);
    assert(prog2.size() >= 2);
    pager.flush();
    return 0;
}
