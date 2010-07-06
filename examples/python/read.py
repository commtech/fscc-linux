#!/usr/bin/python

import fscc

if __name__ == '__main__':
    port = fscc.Port('/dev/fscc0', 'rb')

    print(port.read(4096))

    port.close()
