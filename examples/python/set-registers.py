#!/usr/bin/python

import fscc

if __name__ == '__main__':
    port = fscc.Port('/dev/fscc0', 'wb')

    port.registers.CCR0 = 0x0011201c
    port.registers.CCR1 = 0x00000018
    port.registers.CCR2 = 0x00000000
