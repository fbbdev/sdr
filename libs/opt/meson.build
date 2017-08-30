opt_incl = include_directories('..')
opt_library = static_library('opt', 'opt.cpp')

opt_lib = declare_dependency(include_directories: opt_incl,
                             link_with: opt_library,
                             dependencies: math_lib)
