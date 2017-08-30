project('sdr', 'cpp', 'c',
        default_options: ['c_std=c11', 'cpp_std=c++14', 'b_lto=true'])

# Configuration
cpp = meson.get_compiler('cpp')

vector = get_option('vector')
if vector != ''
    add_project_arguments(vector, language: ['c', 'cpp'])
    add_project_link_arguments(vector, language: ['c', 'cpp'])
endif

release_build = (get_option('buildtype') == 'release')

relax = '-w'

add_project_link_arguments('-fuse-ld=gold',
                           language: ['c', 'cpp'])

if release_build
    add_project_arguments('-ffast-math', language: ['c', 'cpp'])
    add_project_link_arguments('-Wl,-O3', '-O3', '-ffast-math',
                               language: ['c', 'cpp'])
endif
if cpp.get_id() != 'clang'
    add_project_link_arguments('-fuse-linker-plugin',
                               language: ['c', 'cpp'])
endif

# Dependencies
dl_lib = cpp.find_library('dl', required: false)
math_lib = cpp.find_library('m', required: false)
thread_lib = dependency('threads')

# Third-party libraries
subdir('libs')

# Include dirs
subdir('include')

# Libraries
subdir('lib/sdr')
subdir('lib/ui')

# Blocks
subdir('blocks')
