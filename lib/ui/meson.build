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

ui_library = shared_library('sdr-ui',
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
