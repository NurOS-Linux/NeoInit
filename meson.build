project('raesir', 'c',
  version : '0.1.0-alpha2',
  default_options : [
    'c_std=c11',
    'warning_level=2',
  ]
)

add_project_arguments('-D_GNU_SOURCE', language : 'c')

default_path = get_option('default_path')
if default_path != ''
  add_project_arguments('-DDEFAULT_PATH="' + default_path + '"', language : 'c')
endif

services_dir = get_option('services_dir')
add_project_arguments('-DSERVICES_DIR="' + services_dir + '"', language : 'c')

if get_option('container_detection')
  add_project_arguments('-DENABLE_CONTAINER_DETECTION', language : 'c')
endif

if get_option('systemd_compat')
  add_project_arguments('-DRAESIR_SYSTEMD_COMPAT', language : 'c')
endif

if get_option('runit_compat')
  add_project_arguments('-DRAESIR_RUNIT_COMPAT', language : 'c')
endif

raesir_inc = include_directories('include')

systemd_module_sources = []
if get_option('systemd_compat')
  systemd_module_sources = files('modules/systemd/service_sd.c')
endif

runit_module_sources = []
if get_option('runit_compat')
  runit_module_sources = files('modules/runit/service_runit.c')
endif

subdir('src')
subdir('tests')
