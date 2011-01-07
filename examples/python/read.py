#!/usr/bin/python

from fscc import port as fscc_port

if __name__ == '__main__':
    port = fscc_port.Port('/dev/fscc0', 'rb')

    print(port.read(4096))

    port.close()
