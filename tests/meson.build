gtest_dep = dependency('gtest', main: true, required: false)

if gtest_dep.found()
  tests = [
    'pager_tests.cpp',
    'varint_tests.cpp',
    'record_tests.cpp',
    'btree_tests.cpp',
    'vm_tests.cpp',
    'parser_tests.cpp',
    'codegen_tests.cpp',
    'integration_tests.cpp',
  ]

  foreach t : tests
    name = t.split('.')[0]
    exe = executable(name, t, dependencies: [tinydb_dep, gtest_dep])
    test(name, exe)
  endforeach
endif
