#include "tinydb/parser.hpp"
#include "tinydb/codegen.hpp"
#include "tinydb/catalog.hpp"
#include "tinydb/vm.hpp"
#include <cassert>

int main() {
    tinydb::Catalog cat; cat.create_table("t", 1u);
    auto ast = tinydb::parse("SELECT a,b FROM t");
    auto prog = tinydb::codegen(*ast, cat);
    tinydb::VM vm;
    int rc = vm.run(prog);
    assert(rc == 0);
    return 0;
}

