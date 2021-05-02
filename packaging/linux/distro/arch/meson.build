
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

compiler = meson.get_compiler('c')

if compiler.get_id() == 'gcc'
    build_args += ['-Os']
elif compiler.get_id() == 'clang'
    build_args += ['-Oz']
endif

build_args += [
    '-DPROJECT_NAME=' + meson.project_name(),
    '-DPROJECT_VERSION=' + meson.project_version(),
]

message('@0@: @1@'.format(compiler.get_id(), build_args))

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

full_path = run_command('basename', shared_object.full_path()).stdout().strip()
target_name = 'stripped @0@'.format(full_path)

project_target = custom_target(
    target_name,
    input: shared_object,
    output: 'none',
    command: ['strip', '-R', '.comment', '@INPUT@'],
    build_by_default: true,
)
