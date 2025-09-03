test_srcs = [
  'pager_tests.cpp',
  'varint_tests.cpp',
  'record_tests.cpp',
  'btree_tests.cpp',
  'vm_tests.cpp',
  'parser_tests.cpp',
  'codegen_tests.cpp',
  'integration_tests.cpp'
]

foreach t : test_srcs
  name = t.split('.')[0]
  exe = executable('test_' + name, t,
    include_directories: inc,
    dependencies: tinydb_dep
  )
  test(name, exe)
endforeach

