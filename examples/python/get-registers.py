#!/usr/bin/python

import fscc

if __name__ == '__main__':
    port = fscc.Port('/dev/fscc0', 'wb')

    port.CCR0 = fscc.FSCC_UPDATE_VALUE
    port.CCR1 = fscc.FSCC_UPDATE_VALUE
    port.CCR2 = fscc.FSCC_UPDATE_VALUE

    port.get_registers()

    print(("0x%08x" % port.CCR0))
    print(("0x%08x" % port.CCR1))
    print(("0x%08x" % port.CCR2))
