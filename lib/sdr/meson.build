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

sdr_library = static_library('sdr', 'stream.cpp',
                             override_options: ['cpp_std=gnu++14'],
                             include_directories: sdr_incl,
                             dependencies: [kfr_lib, opt_lib,
                                            math_lib, thread_lib])

sdr_lib = declare_dependency(include_directories: sdr_incl,
                             link_with: sdr_library,
                             dependencies: [kfr_lib, opt_lib,
                                            math_lib, thread_lib])
