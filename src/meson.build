liburiparser_gobject_version_h = configure_file(
  input: 'liburiparser-gobject-version.h.in',
  output: 'liburiparser-gobject-version.h',
  configuration: version_conf,
)

liburiparser_gobject_sources = [
  'liburiparser-gobject-version.c',
  'upgerror.c',
  'upguri.c',
]

liburiparser_gobject_headers = [
  'liburiparser-gobject.h',
  liburiparser_gobject_version_h,
  'upgerror.h',
  'upguri.h',
]

liburiparser_gobject_sources += gnome.mkenums_simple('upgenums', sources: liburiparser_gobject_headers)

liburiparser_gobject_lib = library('uriparser-gobject-' + version_split[0],
  liburiparser_gobject_sources,
  dependencies: deps,
  install: true,
)

upg_pkg = declare_dependency(
  link_with: liburiparser_gobject_lib,
  dependencies: deps,
)

install_headers(liburiparser_gobject_headers, subdir: 'liburiparser-gobject')

import('pkgconfig').generate(
  description: 'A GObject/GLib wrapper over liburiparser',
  libraries: upg_pkg,
  name: 'liburiparser-gobject',
  filebase: 'liburiparser-gobject-' + version_split[0],
  version: meson.project_version(),
  subdirs: 'liburiparser-gobject',
  requires: deps,
)

gir = gnome.generate_gir(liburiparser_gobject_lib,
  sources: [ liburiparser_gobject_headers, liburiparser_gobject_sources ],
  header: 'liburiparser-gobject/liburiparser-gobject.h',
  identifier_prefix: 'Upg',
  nsversion: version_split[0],
  namespace: 'UriParser',
  install: true,
  link_with: liburiparser_gobject_lib,
)

gnome.generate_vapi('liburiparser-gobject-' + version_split[0],
  sources: [gir[0]],
  packages: ['glib-2.0'],
  install: true,
)
