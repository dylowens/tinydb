#pragma once
#include <memory>
#include <vector>
#include "tinydb/parser.hpp"
#include "tinydb/vm.hpp"
#include "tinydb/catalog.hpp"

namespace tinydb {

std::vector<Instr> codegen(const ASTNode& ast, const Catalog& cat);

} // namespace tinydb

