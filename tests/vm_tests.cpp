#include "tinydb/vm.hpp"
#include <cassert>

int main() {
    tinydb::VM vm;
    std::vector<tinydb::Instr> prog{{tinydb::Op::Halt,0,0,0,{}}};
    int rc = vm.run(prog);
    assert(rc == 0);
    return 0;
}

