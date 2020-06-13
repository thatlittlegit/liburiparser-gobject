project('liburiparser-gobject', 'c',
        version: '0.1.0',
  meson_version: '>= 0.50.0',
)

gnome = import('gnome')

uriparser = dependency('liburiparser')

deps = [
  dependency('glib-2.0', version: '>= 2'),
  dependency('gobject-2.0', version: '>= 2'),
  uriparser,
]

config_h = configuration_data()
config_h.set_quoted('PACKAGE_VERSION', meson.project_version())
configure_file(
  output: 'liburiparser-gobject-config.h',
  configuration: config_h,
)
add_project_arguments([
  '-I' + meson.build_root(),
], language: 'c')

version_conf = configuration_data()
version_split = meson.project_version().split('.')
version_conf.set('VERSION', meson.project_version())
version_conf.set('MAJOR_VERSION', version_split[0])
version_conf.set('MINOR_VERSION', version_split[1])
version_conf.set('PATCH_VERSION', version_split[2])

subdir('src')
subdir('docs')
subdir('tests')
