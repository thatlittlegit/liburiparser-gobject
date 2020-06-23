project('liburiparser-gobject', 'c',
        version: '0.1.0',
  meson_version: '>= 0.50.0',
)

gnome = import('gnome')

deps = [
  dependency('liburiparser', version: '>= 0.9'),
  dependency('glib-2.0', version: '>= 2.60'),
  dependency('gobject-2.0', version: '>= 2'),
]

add_global_arguments('-DLIBURIPARSER_GOBJECT_COMPILATION',
  language: 'c',
)

version_conf = configuration_data()
version_split = meson.project_version().split('.')
version_conf.set('VERSION', meson.project_version())
version_conf.set('MAJOR_VERSION', version_split[0])
version_conf.set('MINOR_VERSION', version_split[1])
version_conf.set('PATCH_VERSION', version_split[2])

subdir('src')
subdir('docs')
subdir('tests')
