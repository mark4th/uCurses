
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
    '-Wno-extra-semi',
    '-std=gnu17',
    '-pipe',
    '-fno-align-functions',
]

usable_pythons = [ 'python3.12', 'python3.11', 'python3.10', 'python3.9', 'python3.8', 'python3.7', 'python3' ]

python = find_program(usable_pythons, required: true, native: true).path()

source_dir = run_command( python, '-c', 'import os; print(os.getcwd(), end="");', check: true ).stdout()

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

find_test_src = '''from glob import glob;print('\n'.join(glob('src/TEST/*.c')))'''
src_test_sources = run_command(python, '-c', find_test_src, check: true).stdout().strip().split('\n')

find_src = '''from glob import glob;print('\n'.join(glob('src/*.c')))'''

sources = run_command(python, '-c', find_src, check: true)

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


if compiler.get_id() == 'clang'
    test_build_args = build_args

    test_arg_clang = [
        '-fno-inline', '-Wall', '-Werror', '-pipe', '-fno-align-functions', '-Wno-error=pedantic', '-Wno-pedantic',
    ]

    test_linker_args = ['-L/opt/chordlang.io/lib', '-lchordtest', '-lunwind', '-ldl']

    test_build_args_c = test_build_args + [
        '-Oz', '-std=gnu17', '-fblocks', '-I/opt/chordlang.io/include'  ]
    linker_args_test = test_linker_args + [ '-gdwarf', '-fprofile-instr-generate', '-fcoverage-mapping',  ]
    build_args_test_c = test_build_args_c + [ '-D__TESTING__=1', '-gdwarf', '-fprofile-instr-generate', '-fcoverage-mapping', ]
endif


test_src = executable(
    'test-' + meson.project_name() + '.src.app',
    src_test_sources,
    install: false,
    link_args: linker_args_test,
    c_args: build_args_test_c,
)
