
project(
    'uCurses',
    'c',
    version : '0.0.2',
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
    build_args += ['-Os']
elif compiler.get_id() == 'clang'
    build_args += ['-Oz']
endif

build_args += [
    '-DPROJECT_NAME=' + meson.project_name(),
    '-DPROJECT_VERSION=' + meson.project_version(),
]

message('@0@: @1@'.format(compiler.get_id(), build_args))

linker_args = ['-Wl,--entry=entry']

sources = run_command('bash', '-c', '''
    ln -s ${PROJECT_ROOT}/src ${ARCH_MESON}/src
    ln -s ${PROJECT_ROOT}/h ${ARCH_MESON}/h
    cd ${ARCH_MESON} && find src/ -name *.c
''')

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

project_target = custom_target(
    target_name,
    input: shared_object,
    output: 'none',
    command: ['strip', '-R', '.comment', '@INPUT@'],
    build_by_default: true,
)
