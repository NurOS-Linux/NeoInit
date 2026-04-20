# Core logic for testing
neoinit_lib_sources = files(
  'launcher.c',
  'log.c',
  'mount.c',
  'reap.c',
  'service.c',
  'service_yaml.c',
  'signal_handler.c',
  'control.c',
  'registry.c'
) + systemd_module_sources + runit_module_sources

neoinit_lib = static_library('neoinit_lib',
  neoinit_lib_sources,
  include_directories : neoinit_inc,
  install : false
)

# Main executable
executable('neoinit',
  'main.c',
  link_with           : neoinit_lib,
  include_directories : neoinit_inc,
  install             : true
)
