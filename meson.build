project(
    'cz-xdgkit',
    'c','cpp',
    version : run_command('cat', files('./VERSION'), check : false).stdout().strip(),
    meson_version: '>= 0.62.0',
    default_options: [
        'warning_level=2',
        'buildtype=release',
        'cpp_std=c++20'])

NAME = 'XDGKit'
VERSION_MAJOR = meson.project_version().split('.')[0]
VERSION_MINOR = meson.project_version().split('.')[1]
VERSION_PATCH = meson.project_version().split('.')[2]
VERSION_BUILD = run_command('cat', './BUILD', check : false).stdout()
HEADERS_INSTALL_DIR = join_paths(get_option('prefix'), get_option('includedir'), 'CZ/XDG')
CONF_FILE_NAME = 'CZXDGVersion.h'

conf = configuration_data()
conf.set('VERSION_MAJOR', VERSION_MAJOR)
conf.set('VERSION_MINOR', VERSION_MINOR)
conf.set('VERSION_PATCH', VERSION_PATCH)
conf_file = configure_file(
    input : 'src/CZ/XDG/' + CONF_FILE_NAME +'.in',
    output : CONF_FILE_NAME,
    configuration : conf)

install_data(
    conf_file,
    install_dir: HEADERS_INSTALL_DIR)

# -------------- DEPENDENCIES --------------

cpp         = meson.get_compiler('cpp')
pkg         = import('pkgconfig')
cz_core_dep = dependency('cz-core', fallback:['cz-core', 'cz_core_dep'])

# -------------- HEADERS --------------

header_dirs = [
    ['./src/CZ/XDG', '']
]

foreach header_dir : header_dirs
    header_files = run_command('find', header_dir[0], '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(header_files, install_dir : join_paths(HEADERS_INSTALL_DIR, header_dir[1]))
endforeach

# -------------- LIBRARY --------------

cz_xdgkit = library(
    'cz-xdgkit',
    sources : run_command('find', './src/CZ/XDG', '-type', 'f', '-name', '*[.cpp,.c]', check : false).stdout().strip().split('\n'),
    include_directories : ['./src'],
    dependencies : [cz_core_dep],
    soversion: VERSION_MAJOR,
    install : true)

cz_xdgkit_dep = declare_dependency(
    dependencies: [cz_core_dep],
    include_directories : ['./src/', './src/CZ'],
    link_with : cz_xdgkit)

pkg.generate(
    cz_xdgkit,
    name: 'XDGKit',
    description: 'C++ XDG utilities kit.',
    version: meson.project_version(),
    subdirs: ['CZ/XDG'],
    filebase: 'cz-xdgkit')

# -------------- EXAMPLES --------------

if get_option('build_examples')
    subdir('src/examples/cz-xdgkit-icons-example')
endif

if get_option('build_icon_theme_indexer')
    subdir('src/cz-xdgkit-icon-theme-indexer')
endif
