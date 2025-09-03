#include "tinydb/parser.hpp"
#include <cassert>
#include <memory>

int main() {
    auto n1 = tinydb::parse("CREATE TABLE t(a INT, b TEXT)");
    auto n2 = tinydb::parse("INSERT INTO t VALUES(1,'x')");
    auto n3 = tinydb::parse("SELECT a,b FROM t");
    assert(n1 && n2 && n3);
    return 0;
}

