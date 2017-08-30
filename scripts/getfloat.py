#!/usr/bin/env python3

import sys
import struct

if __name__ == '__main__':
    while True:
        try:
            print("float: ", end='', file=sys.stderr)
            f = float(input())
            sys.stdout.buffer.write(struct.pack('f', f))
        except EOFError:
            break
