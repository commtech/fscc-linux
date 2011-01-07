#!/usr/bin/python

from fscc import port as fscc_port

if __name__ == '__main__':
    port = fscc_port.Port('/dev/fscc0', 'wb')

    port.registers.CCR0 = 0x0011201c
    port.registers.CCR1 = 0x00000018
    port.registers.CCR2 = 0x00000000
