tinydb_sources = files(
  'pager.cpp', 'storage.cpp', 'varint.cpp', 'record.cpp',
  'btree.cpp', 'catalog.cpp', 'vm.cpp', 'parser.cpp', 'codegen.cpp', 'ast.cpp',
  'repl.cpp', 'wasm_shim.cpp'
)

tinydb_lib = static_library('tinydb',
  sources: tinydb_sources,
  include_directories: inc,
  install: false
)

tinydb_dep = declare_dependency(
  include_directories: inc,
  link_with: tinydb_lib
)

