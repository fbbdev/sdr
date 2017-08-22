project('sdr', 'cpp', 'c',
        default_options: ['c_std=c11', 'cpp_std=c++14', 'b_lto=true'])

cpp = meson.get_compiler('cpp')
math_lib = cpp.find_library('m', required: false)
thread_lib = dependency('threads')

vector = get_option('vector')
if vector != ''
    add_global_arguments(vector, language: ['c', 'cpp'])
    add_global_link_arguments(vector, language: ['c', 'cpp'])
endif

if cpp.get_id() == 'msvc'
    relax = '/w'
else
    relax = '-w'

    add_global_link_arguments('-fuse-ld=gold',
                              language: ['c', 'cpp'])

    if get_option('buildtype') == 'release'
        add_global_link_arguments('-Wl,-O3', '-O3',
                                  language: ['c', 'cpp'])
    endif
    if cpp.get_id() != 'clang'
        add_global_link_arguments('-fuse-linker-plugin',
                                  language: ['c', 'cpp'])
    endif
endif

subdir('ui')
subdir('stream')

block_deps = [math_lib, thread_lib]

blocks = [
    ['constellation', [ui_lib]],
    ['signal'],
    ['stream_debug'],
    ['throttle'],
]

foreach b : blocks
    if b.length() >= 2
        libs = b[1]
    else
        libs = []
    endif

    executable(b[0], b[0] + '.cpp',
               link_with: [stream_lib, libs],
               dependencies: block_deps)
endforeach