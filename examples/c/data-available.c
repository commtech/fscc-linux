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

#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* close */
#include <stdio.h> /* fprintf, perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h> /* memset */
#include <sys/select.h> /* fd_set, FD_ZERO, FD_SET, FD_ISSET, select */

/*
	This is a simple example showing how to use the select() function to check
	if there is any available data at a port.

*/

int main(void)
{
	fd_set fds;
	struct timeval timeout;
	int port_fd = 0;

	fprintf(stdout, "WARNING (please read)\n");
	fprintf(stdout, "--------------------------------------------------\n");
	fprintf(stdout, "This limited example is for illustrative use only.\n" \
			"Do not use this code in a production environment\n" \
			"without adding proper error checking.\n\n");

	port_fd = open("/dev/fscc0", O_RDWR);

	if (port_fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	memset(&timeout, 0, sizeof(timeout));
	FD_ZERO(&fds);	
	
	/* Set a timeout value of 2 seconds to look for data. */
	timeout.tv_sec = 2;

	/* Specify our file descriptor is what we want to look at. */
	FD_SET(port_fd, &fds);
	
	select(port_fd + 1, &fds, NULL, NULL, &timeout);
	
	fprintf(stdout, "%i\n", FD_ISSET(port_fd, &fds));

	close(port_fd);

	return EXIT_SUCCESS;
}
