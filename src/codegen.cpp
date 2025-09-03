#include "tinydb/codegen.hpp"
#include "tinydb/record.hpp"
#include <type_traits>

namespace tinydb {
namespace {
Value parse_value(const std::string& s) {
    Value v;
    if (!s.empty() && s.front() == '\'' && s.back() == '\'') {
        v.tag = ColTag::TEXT;
        v.s = s.substr(1, s.size() - 2);
    } else {
        v.tag = ColTag::INT;
        v.i = std::stoll(s);
    }
    return v;
}
}

std::vector<Instr> codegen(const ASTNode& ast, const Catalog& cat) {
    std::vector<Instr> p;
    if (auto ins = dynamic_cast<const ASTInsert*>(&ast)) {
        const TableInfo* ti = cat.lookup(ins->table);
        if (!ti) { p.push_back({Op::Halt,0,0,0,{}}); return p; }
        std::vector<Value> vals;
        for (auto& s : ins->values) vals.push_back(parse_value(s));
        auto row = encode_row(vals);
        p.push_back({Op::Insert, static_cast<int>(ti->root),0,0,
                     std::string(reinterpret_cast<const char*>(row.data()), row.size())});
        p.push_back({Op::Halt,0,0,0,{}});
        return p;
    } else if (auto sel = dynamic_cast<const ASTSelect*>(&ast)) {
        const TableInfo* ti = cat.lookup(sel->table);
        if (!ti) { p.push_back({Op::Halt,0,0,0,{}}); return p; }
        p.push_back({Op::OpenRead,0,static_cast<int>(ti->root),0,{}}); // 0
        if (sel->where_rowid) {
            p.push_back({Op::Integer, static_cast<int>(sel->rowid),0,0,{}}); //1
            p.push_back({Op::SeekGE,0,0,0,{}}); //2 fixup
            p.push_back({Op::Column,0,-1,0,{}}); //3
            int ncols = (sel->cols.size()==1 && sel->cols[0]=="*") ? 0 : static_cast<int>(sel->cols.size());
            p.push_back({Op::ResultRow,0,ncols,0,{}}); //4
            p.push_back({Op::Halt,0,0,0,{}}); //5
            p[2].p2 = static_cast<int>(p.size()-1);
        } else {
            p.push_back({Op::Rewind,0,0,0,{}}); //1 fixup
            p.push_back({Op::Column,0,-1,0,{}}); //2 loop start
            int ncols = (sel->cols.size()==1 && sel->cols[0]=="*") ? 0 : static_cast<int>(sel->cols.size());
            p.push_back({Op::ResultRow,0,ncols,0,{}}); //3
            p.push_back({Op::Next,0,0,0,{}}); //4 fixup
            p.push_back({Op::Halt,0,0,0,{}}); //5
            p[1].p2 = static_cast<int>(p.size()-1);
            p[4].p2 = 2; // loop back to Column
        }
        return p;
    }
    p.push_back({Op::Halt,0,0,0,{}});
    return p;
}

} // namespace tinydb

