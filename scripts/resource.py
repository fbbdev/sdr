#!/usr/bin/env python3
#
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

import sys

HEADER_START = """// GENERATED FROM {}
#pragma once

#include <cstdint>

namespace
{{
    const std::uint8_t {}[] = {{
"""

HEADER_END = """    };
} /* namespace */
"""

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: {} FILE ID".format(sys.argv[0]), file=sys.stderr)

    filename = sys.argv[1]
    identifier = sys.argv[2]

    with open(filename, 'rb') as f:
        print(HEADER_START.format(filename, identifier), end='')

        for chunk in iter(lambda: f.read(12), b''):
            line = "        " + " ".join("0x{:0>2x},".format(b) for b in chunk)
            print(line)

        print(HEADER_END, end='')
