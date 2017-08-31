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

nanovg_incl = include_directories('.')
nanovg_library = static_library('nanovg', 'nanovg.c', c_args: [relax],
                                dependencies: math_lib)

nanovg_lib = declare_dependency(include_directories: nanovg_incl,
                                link_with: nanovg_library,
                                dependencies: math_lib)
