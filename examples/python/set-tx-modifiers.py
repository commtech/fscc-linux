#!/usr/bin/python

"""
    This example requires pyfscc. You can download pyfscc by using Mercurial.

    > hg clone https://pyfscc.googlecode.com/hg/ pyfscc

    Or manually downloading the tarball from the following location.

    http://code.google.com/p/pyfscc/downloads/list
"""

"""
	This is a simple example showing how to change the transmit type for each 
	port.

	Valid TX_MODIFIERS are:
	---------------------------------------------------------------------------
	XF - Normal transmit - disable modifiers
	XREP - Transmit repeat
	TXT - Transmit on timer
	TXEXT - Transmit on external signal
	
"""

from fscc import port as fscc_port
from fscc.port import TXT, XREP, XF

if __name__ == '__main__':
    port = fscc_port.Port('/dev/fscc0', 'rb')
    
    port.tx_modifiers = TXT | XREP
    port.tx_modifiers = XF # disable modifiers

    port.close()
