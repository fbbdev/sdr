cc = meson.get_compiler('c')
dl_lib = cc.find_library('dl', required: false)

gl = dependency('gl')
glfw = dependency('glfw3')

ui_library = shared_library('sdr_ui',
    'ui.cpp',
    'glad/glad.c',
    'nanovg/nanovg.c',
    'nuklear/nuklear.c',
    c_args: [relax],
    dependencies: [
        dl_lib,
        math_lib,
        gl,
        glfw,
    ],
    include_directories: [
        sdr_incl,
        include_directories('../../include/sdr/ui')
    ])

ui_lib = declare_dependency(include_directories: ui_incl,
                            link_with: ui_library,
                            dependencies: [math_lib])

executable('ui_test', 'ui_test.cpp',
           dependencies: [ui_lib, glfw])
