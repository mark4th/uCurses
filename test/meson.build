# test/meson.build  — Unity-based unit tests
# built and run with 'ninja test' / 'make tests'
# -----------------------------------------------------------------------

unity_src = files('unity/unity.c')
test_inc   = include_directories('../h', 'unity')
test_args  = [
    '-std=gnu17', '-Wall', '-Wextra', '-g', '-O0',
    '-DUC_MENUS', '-DUC_JSON', '-DUC_STATUS', '-DUC_WIDGETS',
]

ucurses_test_dep = declare_dependency(
    link_with           : shared_object,
    include_directories : test_inc,
)

# -----------------------------------------------------------------------
# helper to define a test executable and register it

foreach t : [
    ['test_smoke',  ['test_smoke.c']],
    ['test_fnv',    ['test_fnv.c']],
    ['test_utf8',   ['test_utf8.c']],
    ['test_list',   ['test_list.c']],
    ['test_window_wide', ['test_window_wide.c']],
]
    exe = executable(t[0], t[1] + [unity_src],
        dependencies     : ucurses_test_dep,
        c_args           : test_args,
        build_by_default : false,
        install          : false,
    )
    test(t[0], exe)
endforeach
