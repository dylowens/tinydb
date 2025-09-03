project('tinydb', 'cpp', version: '0.1.0', default_options: ['warning_level=3', 'cpp_std=c++20'])

inc = include_directories('include')

subdir('src')

if get_option('build_cli')
  subdir('cli')
endif

if get_option('build_tests')
  subdir('tests')
endif

if get_option('build_tools')
  subdir('tools')
endif
