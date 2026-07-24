tests_inc = [include_directories('.'), raesir_inc]

tests = {
  'test_log'          : { 'src': 'test_log.c',          'cargs': [] },
  'test_mount'        : { 'src': 'test_mount.c',        'cargs': [] },
  'test_reap'         : { 'src': 'test_reap.c',         'cargs': [] },
  'test_argv'         : { 'src': 'test_argv.c',         'cargs': [] },
  'test_order'        : { 'src': 'test_order.c',        'cargs': [] },
  'test_service_toml' : { 'src': 'test_service_toml.c', 'cargs': [] },
  'test_integration'  : { 'src': 'test_integration.c',  'cargs': [] },
}

if get_option('systemd_compat')
  tests += {
    'test_service_sd' : { 'src': 'test_service_sd.c', 'cargs': ['-DRAESIR_SYSTEMD_COMPAT'] },
  }
endif

if get_option('runit_compat')
  tests += {
    'test_service_runit' : { 'src': 'test_service_runit.c', 'cargs': ['-DRAESIR_RUNIT_COMPAT'] },
  }
endif

if get_option('yaml_compat')
  tests += {
    'test_service_yaml' : { 'src': 'test_service_yaml.c', 'cargs': ['-DRAESIR_YAML_COMPAT'] },
  }
endif

foreach name, cfg : tests
  exe = executable(name, cfg['src'],
    c_args              : cfg['cargs'],
    link_with           : raesir_lib,
    include_directories : tests_inc,
  )
  test(name, exe)
endforeach
