#!/usr/bin/python

if __name__ == '__main__':
	port = open('/dev/fscc0', 'wb')
	port.write('Hello world!')
	port.flush()
	port.close()
	
