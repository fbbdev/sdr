sdr_library = static_library('sdr', 'stream.cpp',
                             override_options: ['cpp_std=gnu++14'],
                             include_directories: sdr_incl,
                             dependencies: [kfr_lib, opt_lib,
                                            math_lib, thread_lib])

sdr_lib = declare_dependency(include_directories: sdr_incl,
                             link_with: sdr_library,
                             dependencies: [kfr_lib, opt_lib,
                                            math_lib, thread_lib])
