# test/meson.build  — Unity-based unit tests
# built and run with 'ninja test' / 'make tests'
# -----------------------------------------------------------------------

unity_src = files('unity/unity.c')
test_inc   = include_directories('../h', 'unity')
test_args  = [
    '-std=gnu17', '-Wall', '-Wextra', '-g', '-O0',
]
test_args += feature_args

ucurses_test_dep = declare_dependency(
    link_with           : shared_object,
    include_directories : test_inc,
)

# -----------------------------------------------------------------------
# helper to define a test executable and register it

test_specs = [
    ['test_smoke',  ['test_smoke.c'], false],
    ['test_alloc',  ['test_alloc.c'], false],
    ['test_braille', ['test_braille.c'], false],
    ['test_fnv',    ['test_fnv.c'], false],
    ['test_utf8',   ['test_utf8.c'], false],
    ['test_list',   ['test_list.c'], false],
    ['test_window_wide', ['test_window_wide.c'], false],
    ['test_widget_layout', ['test_widget_layout.c'], not widgets_enabled],
    ['test_widget_horizontal', ['test_widget_horizontal.c'], not widgets_enabled],
]

foreach t : test_specs
    exe = executable(t[0], t[1] + [unity_src],
        dependencies     : ucurses_test_dep,
        c_args           : test_args,
        build_by_default : false,
        install          : false,
    )
    test(t[0], exe, should_fail : t[2])
endforeach
