# tinydb

A tiny, educational SQLite-like database in C++20.

## Build

```bash
meson setup build
meson compile -C build
meson test -C build --print-errorlogs
./build/cli/tinydb
```

## WebAssembly

TinyDB can run in the browser via a small C++ shim that exposes
`tinydb_eval_script` and `tinydb_free`. Compile the sources with
Emscripten, for example:

```bash
em++ -std=c++20 -O2 -Iinclude src/*.cpp -sEXPORTED_FUNCTIONS='["_tinydb_eval_script","_tinydb_free"]' -sALLOW_MEMORY_GROWTH=1 -o tinydb.js
```

From JavaScript you can then evaluate newline-delimited scripts:

```js
const mod = await Module();
const ptr = mod.allocateUTF8(script);
const outPtr = mod._tinydb_eval_script(ptr);
console.log(mod.UTF8ToString(outPtr));
mod._tinydb_free(outPtr);
```

