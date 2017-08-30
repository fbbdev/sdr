gl_lib = dependency('gl')

glad_incl = include_directories('.')
glad_library = static_library('glad', 'glad.c', c_args: [relax],
                              dependencies: [dl_lib, gl_lib])

glad_lib = declare_dependency(include_directories: glad_incl,
                              link_with: glad_library,
                              dependencies: [dl_lib, gl_lib])
