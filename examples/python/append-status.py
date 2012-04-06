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

# This example requires pyfscc. You can find pyfscc in the /include/ directory.

from fscc import port as fscc_port

if __name__ == '__main__':
    port = fscc_port.Port('/dev/fscc0', 'w+')
    
    port.append_status = False
    port.write('Hello world!'.encode())
    
    incoming_data = port.read()
    
    print('append_status = off')
    print('bytes_read', len(incoming_data))
    print('data', incoming_data)
    print('status = 0x????')
    print()
    
    port.append_status = True
    port.write('Hello world!'.encode())
    
    incoming_data = port.read()
    
    print('append_status = on')
    print('bytes_read', len(incoming_data))
    print('data', incoming_data)
    print('status', incoming_data[-2:])

    port.close()
