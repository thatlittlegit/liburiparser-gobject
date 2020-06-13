liburiparser_gobject_version_h = configure_file(
  input: 'liburiparser-gobject-version.h.in',
  output: 'liburiparser-gobject-version.h',
  configuration: version_conf,
)

liburiparser_gobject_sources = [
  'upguri.c',
]

liburiparser_gobject_headers = [
  'liburiparser-gobject.h',
  liburiparser_gobject_version_h,
  'upguri.h',
]

liburiparser_gobject_lib = shared_library('uriparser-gobject-' + version_split[0],
  liburiparser_gobject_sources,
  dependencies: deps,
  install: true,
)

install_headers(liburiparser_gobject_headers, subdir: 'liburiparser-gobject')

import('pkgconfig').generate(
  description: 'A GObject/GLib wrapper over liburiparser',
  libraries: liburiparser_gobject_lib,
  name: 'liburiparser-gobject',
  filebase: 'liburiparser-gobject-' + version_split[0],
  version: meson.project_version(),
  subdirs: 'liburiparser-gobject',
  requires: deps,
)

gir = gnome.generate_gir(liburiparser_gobject_lib,
  sources: [ liburiparser_gobject_headers, liburiparser_gobject_sources ],
  header: 'liburiparser-gobject.h',
  identifier_prefix: 'Upg',
  nsversion: meson.project_version().split('.')[0],
  namespace: 'UriParser',
  install: true,
)

gnome.generate_vapi('liburiparser-gobject',
  sources: [gir[0]],
  packages: ['glib-2.0'],
  install: true,
)
