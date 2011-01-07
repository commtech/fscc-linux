#!/usr/bin/python

from fscc import port as fscc_port

if __name__ == '__main__':
    port = fscc_port.Port('/dev/fscc0', 'wb')

    print(("CCR0 = 0x%08x" % port.registers.CCR0))
    print(("CCR1 = 0x%08x" % port.registers.CCR1))
    print(("CCR2 = 0x%08x" % port.registers.CCR2))
