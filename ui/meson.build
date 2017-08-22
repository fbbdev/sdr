cc = meson.get_compiler('c')
dl_lib = cc.find_library('dl', required: false)

gl = dependency('gl')
glfw = dependency('glfw3')

ui_lib = shared_library('sdr_ui',
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
    ])

executable('ui_test', 'ui_test.cpp',
           link_with: ui_lib, dependencies: [math_lib, glfw])
