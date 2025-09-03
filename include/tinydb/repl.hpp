#pragma once

#include <string>

// Process a single REPL line. Appends any output (including newlines)
// to `out` and returns 0 on success. Non-zero return indicates the
// caller should terminate (e.g. on ".quit").
extern "C" int tinydb_process_line(const std::string& line, std::string& out);
