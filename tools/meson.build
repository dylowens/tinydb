tools = [
  ['fuzz_parser', 'fuzz_parser.cpp'],
  ['dump_db', 'dump_db.cpp'],
  ['inspect_vm', 'inspect_vm.cpp'],
]

foreach t : tools
  executable(t[0], t[1], dependencies: tinydb_dep)
endforeach
