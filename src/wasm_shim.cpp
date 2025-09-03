#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <memory>
#include <cstdio>
#include <cstdlib>

extern "C" {
// Your existing CLI entry points you call inside main.cpp, refactor them if needed:
int tinydb_process_line(const std::string& line, std::string& out); 
// ^ Implement by extracting the REPL's single-line handling from your CLI.
// Should append to `out` (including newlines) and return 0 on success.
}

extern "C" {

// Returns a heap-allocated C string with the combined output.
// Caller (JS) must free with `tinydb_free`.
const char* tinydb_eval_script(const char* script_cstr) {
    if (!script_cstr) return nullptr;
    std::istringstream in(script_cstr);
    std::string line, out;
    while (std::getline(in, line)) {
        // Skip empty lines to be friendly
        if (line.empty()) continue;
        // Process each line using your existing REPL handler
        tinydb_process_line(line, out);
    }
    // Allocate a C-string to return
    char* buf = (char*)malloc(out.size() + 1);
    if (!buf) return nullptr;
    std::memcpy(buf, out.data(), out.size());
    buf[out.size()] = '\0';
    return buf;
}

void tinydb_free(const char* p) {
    free((void*)p);
}

}
