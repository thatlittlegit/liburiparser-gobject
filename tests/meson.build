if get_option('tests') == false
  subdir_done()
endif

tests = [
  'comparison.test.c',
  'copy.test.c',
  'fragments.test.c',
  'hierarchy.test.c',
  'parser.test.c',
  'port.test.c',
  'references.test.c',
  'schemes.test.c',
  'userinfo.test.c',
]

jsonglib = dependency('json-glib-1.0')

foreach test: tests
  test(test,
    executable(test,
      [
        test,
        'common.c',
      ],
      dependencies: [
        jsonglib,
        deps,
      ],
      c_args: '-DJSONFILE="@0@"'.format(meson.current_source_dir() / 'tests.json'),
      link_with: liburiparser_gobject_lib,
      include_directories: '../src',
    ),
    protocol: 'tap',
  )
endforeach
