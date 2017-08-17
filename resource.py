#!/usr/bin/env python3

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
        print("Usage: {} FILE ID".format(sys.argv[0]), file=stderr)

    filename = sys.argv[1]
    identifier = sys.argv[2]

    with open(filename, 'rb') as f:
        print(HEADER_START.format(filename, identifier), end='')

        for chunk in iter(lambda: f.read(12), b''):
            line = "        " + " ".join("0x{:0>2x},".format(b) for b in chunk)
            print(line)

        print(HEADER_END, end='')
