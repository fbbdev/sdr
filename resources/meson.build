python = import('python3').find_python()
resource_compiler = files('../scripts/resource.py')

res_incl = include_directories('..')

resources = [
    'font.ttf',
    'waterfall_palette.png'
]

res_deps = []

foreach res : resources
    name = res.split('.')[0]

    res_tgt = custom_target('resource: ' + res,
        input: [resource_compiler, res],
        output: name + '.hpp',
        capture: true,
        command: [python, '@INPUT@', name.underscorify() + '_data'])

    res_deps += declare_dependency(include_directories: res_incl,
                                   sources: res_tgt)
endforeach

font_res = res_deps[0]
waterfall_palette_res = res_deps[1]
all_res = declare_dependency(dependencies: res_deps)
