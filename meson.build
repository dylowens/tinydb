project('tinydb', 'cpp', version: '0.1.0', default_options: [
  'warning_level=3',
  'cpp_std=c++20'
])

add_project_arguments('-DTINYDB_PAGE_SIZE=4096', language: 'cpp')

if get_option('sanitizers') and get_option('buildtype') == 'debug'
  add_project_arguments(['-fsanitize=address,undefined'], language: 'cpp')
  add_project_link_arguments(['-fsanitize=address,undefined'], language: 'cpp')
endif

inc = include_directories('include')

subdir('src')
subdir('cli')
subdir('tests')
subdir('tools')

