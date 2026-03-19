tests_inc = [include_directories('.'), neoinit_inc]

tests = {
  'test_log'          : { 'src': 'test_log.c',          'cargs': [] },
  'test_mount'        : { 'src': 'test_mount.c',        'cargs': [] },
  'test_reap'         : { 'src': 'test_reap.c',         'cargs': [] },
  'test_argv'         : { 'src': 'test_argv.c',         'cargs': [] },
  'test_service_yaml' : { 'src': 'test_service_yaml.c', 'cargs': [] },
}

if get_option('systemd_compat')
  tests += {
    'test_service_sd' : { 'src': 'test_service_sd.c', 'cargs': ['-DNEOINIT_SYSTEMD_COMPAT'] },
  }
endif

foreach name, cfg : tests
  exe = executable(name, cfg['src'],
    c_args             : cfg['cargs'],
    link_with          : neoinit_lib,
    include_directories : tests_inc,
  )
  test(name, exe)
endforeach
