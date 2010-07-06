#!/usr/bin/python

import fscc

if __name__ == '__main__':
    port = fscc.Port('/dev/fscc0', 'wb')

    port.write('Hello world!'.encode())

    port.close()
