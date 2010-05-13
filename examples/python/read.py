#!/usr/bin/python

if __name__ == '__main__':
	port = open('/dev/fscc0', 'rb')
	print port.read()
	port.close()
	
