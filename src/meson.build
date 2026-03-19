neoinit_c_args = []

neoinit_sources = files(
  'log.c',
  'mount.c',
  'reap.c',
  'signal_handler.c',
  'service.c',
  'service_yaml.c',
  'launcher.c',
)

if get_option('systemd_compat')
  neoinit_sources += files('service_sd.c')
  neoinit_c_args  += '-DNEOINIT_SYSTEMD_COMPAT'
endif

neoinit_lib = static_library('neoinit',
  neoinit_sources,
  c_args             : neoinit_c_args,
  include_directories : neoinit_inc,
)

executable('neoinit',
  'main.c',
  c_args             : neoinit_c_args,
  link_with          : neoinit_lib,
  include_directories : neoinit_inc,
  install            : true,
  install_dir        : '/sbin',
)
