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

gl_lib = dependency('gl')

glad_incl = include_directories('.')
glad_library = static_library('glad', 'glad.c', c_args: [relax],
                              dependencies: [dl_lib, gl_lib])

glad_lib = declare_dependency(include_directories: glad_incl,
                              link_with: glad_library,
                              dependencies: [dl_lib, gl_lib])
