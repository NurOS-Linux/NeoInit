project('servctl', 'c',
  version : '0.1.0-alpha2',
  default_options : [
    'c_std=c11',
    'warning_level=2',
  ]
)

add_project_arguments('-D_GNU_SOURCE', language : 'c')

servctl_inc = include_directories('include')
tests_inc   = include_directories('tests')

executable('servctl',
  'src/main.c',
  'src/client.c',
  include_directories : servctl_inc,
  install : true
)

test_client = executable('test_client',
  'tests/test_client.c',
  'src/client.c',
  include_directories : [servctl_inc, tests_inc],
  dependencies : [dependency('threads')],
)
test('test_client', test_client)
