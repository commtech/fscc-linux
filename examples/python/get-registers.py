#!/usr/bin/python

import fscc

if __name__ == '__main__':
    port = fscc.Port('/dev/fscc0', 'wb')

    print(("CCR0 = 0x%08x" % port.registers.CCR0))
    print(("CCR1 = 0x%08x" % port.registers.CCR1))
    print(("CCR2 = 0x%08x" % port.registers.CCR2))
