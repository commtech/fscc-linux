#!/usr/bin/python

from fscc import port as fscc_port

if __name__ == '__main__':
    port = fscc_port.Port('/dev/fscc0', 'wb')

    port.write('Hello world!'.encode())

    port.close()
