tests = [
  'parser.test.c',
]

foreach test: tests
  test(test, executable(test, test,
    dependencies: deps,
    link_with: liburiparser_gobject_lib,
    include_directories: '../src'
  ))
endforeach
