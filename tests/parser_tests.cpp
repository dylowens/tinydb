#include "tinydb/parser.hpp"
#include <cassert>

int main() {
    using namespace tinydb;
    auto n1 = parse("CREATE TABLE t(a INT, b TEXT)");
    auto n2 = parse("INSERT INTO t VALUES(1,'x')");
    auto n3 = parse("SELECT * FROM t WHERE rowid=1");
    assert(dynamic_cast<ASTCreate*>(n1.get()));
    assert(dynamic_cast<ASTInsert*>(n2.get()));
    auto s = dynamic_cast<ASTSelect*>(n3.get());
    assert(s && s->where_rowid && s->rowid==1);
    assert(!parse("BAD SQL"));
    return 0;
}
