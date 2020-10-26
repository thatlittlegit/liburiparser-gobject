if get_option('demo') == false
  subdir_done()
endif

demo_sources = [
  'wizard.c',
  'propertypair.c',
  import('gnome').compile_resources('demo-resources', 'demo.gresources.xml')
]

executable('uri-wizard', demo_sources,
  install: true,
  dependencies: [upg_pkg, deps, dependency('gtk+-3.0')],
  include_directories: '../src',
  pie: true,
)
