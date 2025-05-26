project(
    'XDGKit',
    'c','cpp',
    version : '0.1.0',
    meson_version: '>= 0.62.0',
    default_options: [
        'warning_level=2',
        'buildtype=release',
        'cpp_std=c++20'])

VERSION_MAJOR = meson.project_version().split('.')[0]
VERSION_MINOR = meson.project_version().split('.')[1]
VERSION_PATCH = meson.project_version().split('.')[2]
VERSION_BUILD = run_command('cat', './BUILD', check : false).stdout()

HEADERS_INSTALL_PATH = join_paths(get_option('prefix'), get_option('includedir'), 'XDGKit')

# -------------- DEPENDENCIES --------------

cpp         = meson.get_compiler('cpp')
pkg         = import('pkgconfig')

# -------------- HEADERS --------------

header_dirs = [
    ['./src/XDGKit', ''],
]

foreach header_dir : header_dirs
    header_files = run_command('find', header_dir[0], '-type', 'f', '-name', '*.h', '-maxdepth', '1', check : false).stdout().strip().split('\n')
    install_headers(header_files, install_dir : join_paths(HEADERS_INSTALL_PATH, header_dir[1]))
endforeach

# -------------- LIBRARY --------------

xdgkit = library(
    'XDGKit',
    sources : run_command('find', './src/XDGKit', '-type', 'f', '-name', '*[.cpp,.c]', check : false).stdout().strip().split('\n'),
    include_directories : ['./src'],
    dependencies : [
    ],
    soversion: VERSION_MAJOR,
    install : true)

xdgkit_dep = declare_dependency(
    dependencies: [],
    include_directories : ['./src'],
    link_with : xdgkit)

pkg.generate(
    xdgkit,
    name: 'XDGKit',
    description: 'C++ XDG utilities kit.',
    version: meson.project_version(),
    filebase: 'XDGKit')

# -------------- EXAMPLES --------------

if get_option('build_examples')
    subdir('src/examples/icons')
endif

if get_option('build_icon_theme_indexer')
    subdir('src/xdgkit-icon-theme-indexer')
endif
