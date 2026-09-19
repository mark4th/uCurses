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
    ['test_border', ['test_border.c'], false],
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

# -----------------------------------------------------------------------
# uC_key_sm.c is a hidden symbol, so it cannot be reached through the shared
# library.  compile it straight into this test and let the test supply its
# own ti_vars.

test_key_sm = executable('test_key_sm',
    ['test_key_sm.c', '../src/keys/uC_key_sm.c', unity_src],
    include_directories : test_inc,
    c_args              : test_args,
    build_by_default    : false,
    install             : false,
)
test('test_key_sm', test_key_sm)

# streaming read path: fd source + sm_run over a pipe (fds 0/1).  compiles
# the read layer and the SM straight in and supplies its own ti_vars.

# openpty(): the test gives itself a tty on fd 0 so uC_init_terminal() has
# something to configure whatever the suite was launched from.

test_ctrl_c = executable('test_ctrl_c',
    ['test_ctrl_c.c', unity_src],
    dependencies        : ucurses_test_dep,
    c_args              : test_args,
    link_args           : ['-lutil'],
    build_by_default    : false,
    install             : false,
)
test('test_ctrl_c', test_ctrl_c)

# -----------------------------------------------------------------------

test_key_stream = executable('test_key_stream',
    ['test_key_stream.c', '../src/keys/uC_key_read.c',
     '../src/keys/uC_key_sm.c', unity_src],
    include_directories : test_inc,
    # fds 0/1 are duped onto the pipe for the fake tty, so Unity must report
    # on fd 2 (via unity_putc) or its output would be swallowed by the pipe.
    c_args              : test_args + [
        '-DUNITY_OUTPUT_CHAR(a)=unity_putc(a)',
        '-DUNITY_OUTPUT_CHAR_HEADER_DECLARATION=unity_putc(int c)',
    ],
    build_by_default    : false,
    install             : false,
)
test('test_key_stream', test_key_stream)
