tools = [
  ['dump_db',      'dump_db.cpp'],
  ['inspect_vm',   'inspect_vm.cpp'],
  ['fuzz_parser',  'fuzz_parser.cpp']
]

foreach tool : tools
  executable(tool[0], tool[1],
    include_directories: inc,
    dependencies: tinydb_dep,
    install: false
  )
endforeach

