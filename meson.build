
project(
    'uCurses',
    'c',
    version : '1.0',
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
    '-fvisibility=hidden',
    '-I../h'
]

usable_pythons = [ 'python3.12', 'python3.11', 'python3.10', 'python3.9', 'python3.8', 'python3.7', 'python3' ]

python = find_program(usable_pythons, required: true, native: true).path()

source_dir = run_command(python, '-c', 'import os; print(os.getcwd(), end="");', check: true ).stdout()

compiler = meson.get_compiler('c')

if compiler.get_id() == 'gcc'
    build_args += ['-Os', '-Wno-error=attributes']
elif compiler.get_id() == 'clang'
    build_args += ['-Oz']
endif

build_args += [
    '-DPROJECT_NAME="@0@"'.format(meson.project_name()),
    '-DPROJECT_VERSION="@0@"'.format(meson.project_version()),

    # optional stuff

    '-DUC_MENUS', 
    '-DUC_JSON', 
    '-DUC_STATUS',
    '-DUC_WIDGETS'
]

message('@0@: @1@'.format(compiler.get_id(), build_args))


linker_args = ['-Wl,--entry=entry', '-Wl,-z,pack-relative-relocs']

find_src    = '''from glob import glob;print('\n'.join(glob('src/*.c')))'''
find_jsn    = '''from glob import glob;print('\n'.join(glob('src/json/*.c')))'''
find_ti     = '''from glob import glob;print('\n'.join(glob('src/terminfo/*.c')))'''
find_key    = '''from glob import glob;print('\n'.join(glob('src/keys/*.c')))'''
find_ui     = '''from glob import glob;print('\n'.join(glob('src/ui/*.c')))'''
find_menu   = '''from glob import glob;print('\n'.join(glob('src/ui/menus/*.c')))'''
find_wdgt   = '''from glob import glob;print('\n'.join(glob('src/ui/widgets/*.c')))'''
find_util   = '''from glob import glob;print('\n'.join(glob('src/utils/*.c')))'''

sources       = run_command(python, '-c', find_src,  check: true)
source_jsn    = run_command(python, '-c', find_jsn,  check: true)
source_ti     = run_command(python, '-c', find_ti,   check: true)
source_key    = run_command(python, '-c', find_key,  check: true)
source_ui     = run_command(python, '-c', find_ui,   check: true)
source_menu   = run_command(python, '-c', find_menu, check: true)
source_wdgt   = run_command(python, '-c', find_wdgt, check: true)
source_util   = run_command(python, '-c', find_util, check: true)

project_source_files = sources.stdout().strip().split('\n')
json_source_files    = source_jsn.stdout().strip().split('\n')
ti_source_files      = source_ti.stdout().strip().split('\n')
key_source_files     = source_key.stdout().strip().split('\n')
ui_source_files      = source_ui.stdout().strip().split('\n')
menu_source_files    = source_menu.stdout().strip().split('\n')
wdgt_source_files    = source_wdgt.stdout().strip().split('\n')
util_source_files    = source_util.stdout().strip().split('\n')

shared_object = shared_library(
    meson.project_name(),
    project_source_files,
    json_source_files,
    key_source_files,
    ti_source_files,
    ui_source_files,
    menu_source_files,
    wdgt_source_files,
    util_source_files,
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
#            strip -R .comment @INPUT@
            touch @0@
            file @INPUT@ > /proc/$PPID/fd/1
            du -h @INPUT@ > /proc/$PPID/fd/1
        '''.format(dot_stripped)
    ],
    build_by_default: true,
)
