#include "tinydb/vm.hpp"
#include <limits>

namespace tinydb {

int VM::run(const std::vector<Instr>& prog) {
    results_.clear();
    size_t pc = 0;
    while (pc < prog.size()) {
        const Instr& ins = prog[pc];
        switch (ins.op) {
        case Op::OpenRead:
        case Op::OpenWrite: {
            if (!btree_) return 1;
            if (cursors_.size() <= static_cast<size_t>(ins.p1))
                cursors_.resize(ins.p1 + 1);
            cursors_[ins.p1] = btree_->open(static_cast<uint32_t>(ins.p2));
            ++pc;
            break;
        }
        case Op::Rewind: {
            auto& c = cursors_[ins.p1];
            btree_->seek(c, std::numeric_limits<int64_t>::min());
            std::string_view payload = btree_->read_payload(c);
            if (payload.empty()) pc = static_cast<size_t>(ins.p2); else ++pc;
            break;
        }
        case Op::SeekGE: {
            auto& c = cursors_[ins.p1];
            int64_t key = 0;
            if (static_cast<size_t>(ins.p3) < regs_.size()) key = regs_[ins.p3].i;
            bool found = btree_->seek(c, key);
            if (!found) pc = static_cast<size_t>(ins.p2); else ++pc;
            break;
        }
        case Op::Next: {
            auto& c = cursors_[ins.p1];
            bool ok = btree_->next(c);
            if (ok) pc = static_cast<size_t>(ins.p2); else ++pc;
            break;
        }
        case Op::Column: {
            auto& c = cursors_[ins.p1];
            std::string_view payload = btree_->read_payload(c);
            auto row = decode_row(reinterpret_cast<const uint8_t*>(payload.data()), payload.size());
            if (ins.p2 >= 0) {
                if (regs_.size() <= static_cast<size_t>(ins.p3)) regs_.resize(ins.p3 + 1);
                if (static_cast<size_t>(ins.p2) < row.size()) regs_[ins.p3] = row[ins.p2];
            } else {
                last_row_cols_ = row.size();
                if (regs_.size() < last_row_cols_) regs_.resize(last_row_cols_);
                for (size_t i = 0; i < row.size(); ++i) regs_[i] = row[i];
            }
            ++pc;
            break;
        }
        case Op::Integer: {
            if (regs_.size() <= static_cast<size_t>(ins.p2)) regs_.resize(ins.p2 + 1);
            regs_[ins.p2].tag = ColTag::INT;
            regs_[ins.p2].i = ins.p1;
            ++pc;
            break;
        }
        case Op::Insert: {
            if (!btree_) return 1;
            std::string_view payload(ins.p4);
            btree_->insert(static_cast<uint32_t>(ins.p1), {next_rowid_++}, payload);
            ++pc;
            break;
        }
        case Op::ResultRow: {
            int n = ins.p2 == 0 ? static_cast<int>(last_row_cols_) : ins.p2;
            std::vector<Value> row;
            row.reserve(n);
            for (int i = 0; i < n; ++i) {
                if (static_cast<size_t>(ins.p1 + i) < regs_.size())
                    row.push_back(regs_[ins.p1 + i]);
                else
                    row.push_back(Value{});
            }
            results_.push_back(std::move(row));
            ++pc;
            break;
        }
        case Op::Halt:
            return 0;
        }
    }
    return 0;
}

} // namespace tinydb

