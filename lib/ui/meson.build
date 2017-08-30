ui_library = shared_library('sdr_ui',
    'ui.cpp',
    dependencies: [
        font_res,
        glad_lib,
        nanovg_lib,
        nuklear_lib,
        math_lib,
        glfw_lib,
    ],
    include_directories: [
        sdr_incl,
        include_directories('../../include/sdr/ui')
    ])

ui_lib = declare_dependency(include_directories: [
                                ui_incl,
                                glad_incl,
                                nanovg_incl,
                                nuklear_incl
                            ],
                            link_with: ui_library,
                            dependencies: [math_lib])

executable('ui_test', 'ui_test.cpp',
           dependencies: [ui_lib, glfw_lib])
