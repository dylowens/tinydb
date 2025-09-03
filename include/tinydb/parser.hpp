#pragma once
#include <memory>
#include <string>
#include <vector>

namespace tinydb {

struct ASTNode { virtual ~ASTNode() = default; };
struct ASTCreate : ASTNode { std::string table; std::vector<std::pair<std::string,std::string>> cols; };
struct ASTInsert : ASTNode { std::string table; std::vector<std::string> values; };
struct ASTSelect : ASTNode { std::string table; std::vector<std::string> cols; bool where_rowid=false; long long rowid=0; };

std::unique_ptr<ASTNode> parse(const std::string& sql);

} // namespace tinydb

