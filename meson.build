project(
    'argus-git', 'c',
    version: '1.0.0',
    license: 'MIT',
    default_options: [
        'warning_level=3',
        'werror=false',
        'buildtype=debug',
        'c_std=c11',
    ],
    meson_version: '>=1.1.0',
)

cc = meson.get_compiler('c')

# Import Argus library via wrap
argus_dep = dependency(
    'argus',
    fallback: ['argus', 'argus_dep'],
    default_options: ['regex=true', 'buildtype=debug']
)

# Include directories
inc_dirs = include_directories('include')

# Include commands subdirectory
subdir('src/commands')

# Collect all source files
src_files = [
    'src/main.c',
    'src/mock_data.c',
] + commands_sources

# Build executable
executable(
    'git',
    src_files,
    include_directories: inc_dirs,
    dependencies: [argus_dep],
    c_args: ['-DARGUS_DEBUG', '-DARGUS_REGEX'],
    install: true,
)