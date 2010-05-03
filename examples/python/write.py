if __name__ == '__main__':
	port = open('/dev/fscc0', 'w')
	port.write('Hello world!')
	port.flush()
	port.close()
