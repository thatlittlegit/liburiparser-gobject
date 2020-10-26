if get_option('docs') == false
  subdir_done()
endif

configure_file(
  input: 'version.xml.in',
  output: 'version.xml',
  configuration: version_conf
)

gnome.gtkdoc('liburiparser-gobject',
  src_dir: 'src',
  namespace: 'upg',
  install: true,
  dependencies: upg_pkg,
  gobject_typesfile: 'liburiparser-gobject.types',
  scan_args: '--rebuild-types',
  main_xml: 'liburiparser-gobject.xml',
)
