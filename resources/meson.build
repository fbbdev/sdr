# sdr - software-defined radio building blocks for unix pipes
# Copyright (C) 2017 Fabio Massaioli
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

resources = [
    'font.ttf',
    'waterfall_palette.png'
]

res_deps = []

python = import('python3').find_python()
resource_compiler = files('../scripts/resource.py')

res_incl = include_directories('..')

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
