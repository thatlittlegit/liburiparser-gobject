configure_file(
  input: 'version.xml.in',
  output: 'version.xml',
  configuration: version_conf
)

gnome.gtkdoc('liburiparser-gobject',
  src_dir: 'src',
  namespace: 'upg',
  install: true,
  main_xml: 'liburiparser-gobject.xml',
)
