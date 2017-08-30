glfw_lib = dependency('glfw3')

nuklear_incl = include_directories('.')
nuklear_library = static_library('nuklear', 'nuklear.c', c_args: [relax],
                                 dependencies: [glad_lib, glfw_lib, math_lib])

nuklear_lib = declare_dependency(include_directories: nuklear_incl,
                                 link_with: nuklear_library,
                                 dependencies: [glad_lib, glfw_lib, math_lib])
