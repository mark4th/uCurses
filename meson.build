
project(
    'uCurses',
    'c',
    version : '0.0.1',
    default_options : ['warning_level=3'],
    license: 'MIT',
)
project_description = 'TUI app builder for consoles and terminals.'

build_args = [
    '-fno-inline',
    '-Werror',
    '-std=gnu17',
    '-pipe',
    '-fno-align-functions',
]

compiler = meson.get_compiler('c')

if compiler.get_id() == 'gcc'
    build_args += ['-Os', '-Wno-error=attributes']
elif compiler.get_id() == 'clang'
    build_args += ['-Oz']
endif

build_args += [
    '-DPROJECT_NAME="@0@"'.format(meson.project_name()),
    '-DPROJECT_VERSION="@0@"'.format(meson.project_version()),
]

message('@0@: @1@'.format(compiler.get_id(), build_args))

linker_args = ['-Wl,--entry=entry']

sources = run_command('bash', '-c', 'find src/ -name *.c')

project_source_files = sources.stdout().strip().split('\n')

shared_object = shared_library(
    meson.project_name(),
    project_source_files,
    install : true,
    link_args : linker_args,
    c_args : build_args,
    version : meson.project_version(),
    soversion : meson.project_version(),
)

full_path = run_command('basename', shared_object.full_path()).stdout().strip()
target_name = 'stripped @0@'.format(full_path)

dot_stripped = '.stripped'
project_target = custom_target(
    target_name,
    input: shared_object,
    output: dot_stripped,
    command: [
        'bash', '-c',
        '''
            strip -R .comment @INPUT@
            touch @0@
            file @INPUT@ > /proc/$PPID/fd/1
            du -h @INPUT@ > /proc/$PPID/fd/1
        '''.format(dot_stripped)
    ],
    build_by_default: true,
)
