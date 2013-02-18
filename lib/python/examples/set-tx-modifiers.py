"""
	Copyright (C) 2012 Commtech, Inc.

	This file is part of fscc-linux.

	fscc-linux is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	fscc-linux is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with fscc-linux.	If not, see <http://www.gnu.org/licenses/>.

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

# This example requires pyfscc. You can find pyfscc in the /include/ directory.

from fscc import Port
from fscc import TXT, XREP, XF

if __name__ == '__main__':
    port = Port('/dev/fscc0', 'rb')

    port.tx_modifiers = TXT | XREP
    port.tx_modifiers = XF # disable modifiers

    port.close()
