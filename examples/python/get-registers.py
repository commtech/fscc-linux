#!/usr/bin/python

"""
    This example requires pyfscc. You can download pyfscc by using Mercurial.

    > hg clone https://pyfscc.googlecode.com/hg/ pyfscc

    Or manually downloading the tarball from the following location.

    http://code.google.com/p/pyfscc/downloads/list

"""

from fscc import port as fscc_port

if __name__ == '__main__':
    port = fscc_port.Port('/dev/fscc0', 'wb')

    print(("CCR0 = 0x%08x" % port.registers.CCR0))
    print(("CCR1 = 0x%08x" % port.registers.CCR1))
    print(("CCR2 = 0x%08x" % port.registers.CCR2))
