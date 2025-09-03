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
    Pager pager(std::make_unique<FileStorage>("integration.db"));
    BTree bt(pager);
    Catalog cat(pager, bt);
    cat.create_table("t");
    VM vm(bt, cat);
    vm.run(codegen(*parse("INSERT INTO t VALUES(1,'a')"), cat));
    vm.run(codegen(*parse("INSERT INTO t VALUES(2,'b')"), cat));
    vm.run(codegen(*parse("SELECT * FROM t WHERE rowid=2"), cat));
    assert(vm.results().size() == 1);
    assert(vm.results()[0][0].i == 2);
    assert(vm.results()[0][1].s == "b");
    pager.flush();
    return 0;
}
