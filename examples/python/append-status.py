#!/usr/bin/python

"""
    This example requires pyfscc. You can download pyfscc by using Mercurial.

    > hg clone https://pyfscc.googlecode.com/hg/ pyfscc

    Or manually downloading the tarball from the following location.

    http://code.google.com/p/pyfscc/downloads/list

"""

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
