# Core logic for testing
raesir_lib_sources = files(
  'cgroup.c',
  'launcher.c',
  'log.c',
  'mount.c',
  'order.c',
  'reap.c',
  'service.c',
  'service_toml.c',
  'signal_handler.c',
  'control.c',
  'registry.c'
) + systemd_module_sources + runit_module_sources + yaml_module_sources

raesir_lib = static_library('raesir_lib',
  raesir_lib_sources,
  include_directories : raesir_inc,
  install : false
)

# Main executable
executable('raesir',
  'main.c',
  link_with           : raesir_lib,
  include_directories : raesir_inc,
  install             : true
)
