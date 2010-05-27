#!/usr/bin/python

import io

if __name__ == '__main__':
    # The port needs to be treated as a file stream instead of a regular file
    port = io.FileIO('/dev/fscc0', 'rb')

    # Read must be passed a length value. No value will cause you to lose data
    print port.read(4096)

    port.close()
