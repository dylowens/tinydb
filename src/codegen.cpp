#include "tinydb/codegen.hpp"
#include <type_traits>

namespace tinydb {

std::vector<Instr> codegen(const ASTNode& ast, const Catalog&) {
    std::vector<Instr> p;
    // Minimal program just halts (stub)
    p.push_back({Op::Halt, 0,0,0,{}});
    return p;
}

} // namespace tinydb

