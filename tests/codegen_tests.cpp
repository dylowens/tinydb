#include "tinydb/parser.hpp"
#include "tinydb/codegen.hpp"
#include "tinydb/catalog.hpp"
#include <cassert>

int main() {
    tinydb::Catalog cat;
    cat.create_table("t", 1u);
    auto ast = tinydb::parse("SELECT a,b FROM t");
    auto prog = tinydb::codegen(*ast, cat);
    (void)prog;
    return 0;
}

