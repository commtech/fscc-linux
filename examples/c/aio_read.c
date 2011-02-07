/*
	Copyright (C) 2011 Commtech, Inc.

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

*/

#include <fcntl.h> /* open, O_RDONLY */
#include <unistd.h> /* close */
#include <stdio.h> /* fprintf, perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h> /* memset */
#include <aio.h> /* aio_read, aio_error, aio_return */
#include <errno.h> /* EINPROGRESS */

/*
	This is a simple example of using the aio_* series of linux	functions. AIO 
	allows for overlapping I/O operations. If you would like to do other
	operations while waiting on data to arrive take a look at the aio_*
	functions.
	
	NOTE: You may need to link against the retail time library to use the aio_*
          functions e.g. gcc -lrt aio_read.c

*/

int main(void)
{
	ssize_t bytes_read = 0;
	int port_fd = 0;
	char data[20];
	struct aiocb aio;

	fprintf(stdout, "WARNING (please read)\n");
	fprintf(stdout, "--------------------------------------------------\n");
	fprintf(stdout, "This limited example is for illustrative use only.\n" \
			"Do not use this code in a production environment\n" \
			"without adding proper error checking.\n\n");

	port_fd = open("/dev/fscc0", O_RDONLY);

	if (port_fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	memset(&aio, 0, sizeof(aio));
	memset(&data, 0, sizeof(data));

	aio.aio_fildes = port_fd;
	aio.aio_buf = data;
	aio.aio_nbytes = sizeof(data);

	aio_read(&aio);

	while (aio_error(&aio) == EINPROGRESS) {}

	bytes_read = aio_return(&aio);

	fprintf(stdout, "%s\n", data);

	close(port_fd);

	return EXIT_SUCCESS;
}

