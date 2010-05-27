#!/usr/bin/python

import io

if __name__ == '__main__':
    # The port needs to be treated as a file stream instead of a regular file
    port = io.FileIO('/dev/fscc0', 'wb')

    port.write('Hello world!')

    port.close()
