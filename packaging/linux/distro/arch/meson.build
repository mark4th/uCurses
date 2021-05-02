
project(
    'uCurses',
    'c',
    version : '0.0.2',
    default_options : ['warning_level=3']
)
project_description = 'TUI app builder for consoles and terminals.'

project_source_files = [
    'src/border.c',
    'src/color.c',
    'src/json.c',
    'src/json_key.c',
    'src/json_token.c',
    'src/json_value.c',
    'src/keys.c',
    'src/list.c',
    'src/menus.c',
    'src/parse.c',
    'src/reswitch.c',
    'src/screen.c',
    'src/tifile.c',
    'src/ui_json.c',
    'src/utf8.c',
    'src/util.c',
    'src/window.c',
    'src/win_printf.c',
]

build_args = [
    '-Oz',
    '-fpic',
    '-fno-inline',
    '-pedantic',
    '-Wall',
    '-Werror',
    '-Wextra',
    '-std=gnu17',
    '-pipe',
    '-march=native',
    '-g3',
    '-fno-align-functions',
    '-m64',
]

build_args += [
    '-DPROJECT_NAME=' + meson.project_name(),
    '-DPROJECT_VERSION=' + meson.project_version(),
]

linker_args = [
    '-Wl,--entry=entry',
]

shared_object = shared_library(
    meson.project_name(),
    project_source_files,
    install : true,
    link_args : linker_args ,
    c_args : build_args,
    version : meson.project_version(),
    soversion : meson.project_version(),
)

target_name = (
    'stripped lib' + meson.project_name() + '.so.' + meson.project_version()
)

project_target = custom_target(
    target_name,
    input: shared_object,
    output: 'none',
    command: ['strip', '-R', 'comment', '@INPUT@'],
    build_by_default: true,
)
