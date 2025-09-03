#include "tinydb/parser.hpp"
#include "tinydb/codegen.hpp"
#include "tinydb/catalog.hpp"
#include "tinydb/storage.hpp"
#include "tinydb/pager.hpp"
#include "tinydb/btree.hpp"
#include "tinydb/vm.hpp"
#include <cassert>
#include <memory>

int main() {
    using namespace tinydb;
    Pager pager(std::make_unique<FileStorage>("vm.db"));
    BTree bt(pager);
    Catalog cat(pager, bt);
    cat.create_table("t");
    VM vm(bt, cat);
    auto ins = parse("INSERT INTO t VALUES(1,'x')");
    auto prog1 = codegen(*ins, cat);
    vm.run(prog1);
    auto sel = parse("SELECT * FROM t");
    auto prog2 = codegen(*sel, cat);
    vm.run(prog2);
    assert(vm.results().size() == 1);
    assert(vm.results()[0][0].i == 1);
    assert(vm.results()[0][1].s == "x");
    pager.flush();
    return 0;
}
