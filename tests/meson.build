tests = [
  'fragments.test.c',
  'parser.test.c',
]

foreach test: tests
  test(test,
    executable(test,
      [
        test,
        'common.c',
      ],
      dependencies: [
        dependency('json-glib-1.0'),
        deps,
      ],
      c_args: '-DJSONFILE="@0@"'.format(meson.current_source_dir() / 'tests.json'),
      link_with: liburiparser_gobject_lib,
      include_directories: '../src',
    ),
    protocol: 'tap',
  )
endforeach
