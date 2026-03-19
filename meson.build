project('neoinit', 'c',
  version : '0.1.0',
  default_options : [
    'c_std=c11',
    'warning_level=2',
  ]
)

add_project_arguments('-D_GNU_SOURCE', language : 'c')

neoinit_inc = include_directories('include')

subdir('src')
subdir('tests')
