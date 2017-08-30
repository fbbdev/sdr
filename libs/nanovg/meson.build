nanovg_incl = include_directories('.')
nanovg_library = static_library('nanovg', 'nanovg.c', c_args: [relax],
                                dependencies: math_lib)

nanovg_lib = declare_dependency(include_directories: nanovg_incl,
                                link_with: nanovg_library,
                                dependencies: math_lib)
