srcs = [
  'pager.cpp',
  'storage.cpp',
  'varint.cpp',
  'record.cpp',
  'btree.cpp',
  'catalog.cpp',
  'vm.cpp',
  'parser.cpp',
  'codegen.cpp',
  'ast.cpp',
]

libtinydb = static_library('tinydb', srcs, include_directories: inc)
tinydb_dep = declare_dependency(link_with: libtinydb, include_directories: inc)
