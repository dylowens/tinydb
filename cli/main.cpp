#include "tinydb/repl.hpp"
#include <iostream>
#include <string>

int main() {
    std::string line, out;
    std::cout << "tinydb CLI\n";
    while (std::cout << "tinydb> " && std::getline(std::cin, line)) {
        out.clear();
        int rc = tinydb_process_line(line, out);
        if (!out.empty()) std::cout << out;
        if (rc != 0) break;
    }
    return 0;
}
