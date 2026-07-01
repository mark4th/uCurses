# example/meson.build  — uCurses example programs
# not built by default; use 'ninja examples' or 'make examples'
# -----------------------------------------------------------------------

m_dep   = compiler.find_library('m', required : false)
ex_inc  = include_directories('../h', '.')
ex_rpath = '$ORIGIN/../build'
ex_args = [
    '-std=gnu17', '-Wall', '-Wextra', '-g', '-O2',
]
ex_args += feature_args

ucurses_dep = declare_dependency(
    link_with           : shared_object,
    include_directories : ex_inc,
)

# -----------------------------------------------------------------------

ex_demo = executable('demo',
    ['main.c', 'menus.c', 'hello.c'],
    dependencies     : [ucurses_dep, m_dep],
    c_args           : ex_args,
    build_rpath      : ex_rpath,
    build_by_default : false,
    install          : false,
)

# scroll.c is a mouse integration demo.  Keep it out of the keyboard-only
# example set until the mouse path is enabled explicitly.
if mouse_enabled
    ex_scroll = executable('scroll',
        'scroll.c',
        dependencies     : [ucurses_dep, m_dep],
        c_args           : ex_args,
    build_rpath      : ex_rpath,
        build_by_default : false,
        install          : false,
    )
endif

ex_console = executable('console',
    'console.c',
    dependencies     : ucurses_dep,
    c_args           : ex_args,
    build_rpath      : ex_rpath,
    build_by_default : false,
    install          : false,
)

ex_widgets = executable('widgets',
    'widgets.c',
    dependencies     : ucurses_dep,
    c_args           : ex_args,
    build_rpath      : ex_rpath,
    build_by_default : false,
    install          : false,
)

ex_window = executable('window',
    'window.c',
    dependencies     : [ucurses_dep, m_dep],
    c_args           : ex_args,
    build_rpath      : ex_rpath,
    build_by_default : false,
    install          : false,
)

ex_raycast = executable('raycast',
    'raycast.c',
    dependencies     : [ucurses_dep, m_dep],
    c_args           : ex_args,
    build_rpath      : ex_rpath,
    build_by_default : false,
    install          : false,
)

ex_dots = executable('dots',
    ['dots.c', 'shapes.c'],
    dependencies     : [ucurses_dep, m_dep],
    c_args           : ex_args,
    build_rpath      : ex_rpath,
    build_by_default : false,
    install          : false,
)

ex_mandel = executable('mandel',
    'mandel.c',
    dependencies     : [ucurses_dep, m_dep],
    c_args           : ex_args,
    build_rpath      : ex_rpath,
    build_by_default : false,
    install          : false,
)

ex_borders = executable('borders',
    'borders.c',
    dependencies     : ucurses_dep,
    c_args           : ex_args,
    build_rpath      : ex_rpath,
    build_by_default : false,
    install          : false,
)

ex_lion = executable('lion',
    'lion.c',
    dependencies     : ucurses_dep,
    c_args           : ex_args,
    build_rpath      : ex_rpath,
    build_by_default : false,
    install          : false,
)

ex_list_demo = executable('list_demo',
    'list_demo.c',
    dependencies     : ucurses_dep,
    c_args           : ex_args,
    build_rpath      : ex_rpath,
    build_by_default : false,
    install          : false,
)

# -----------------------------------------------------------------------

if mouse_enabled
    alias_target('examples',
        ex_demo, ex_scroll, ex_console, ex_widgets, ex_window,
        ex_raycast, ex_dots, ex_mandel, ex_borders, ex_lion, ex_list_demo,
    )
else
    alias_target('examples',
        ex_demo, ex_console, ex_widgets, ex_window,
        ex_raycast, ex_dots, ex_mandel, ex_borders, ex_lion, ex_list_demo,
    )
endif
