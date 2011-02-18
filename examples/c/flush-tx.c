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

#include <fcntl.h> /* open, O_WRONLY */
#include <unistd.h> /* close */
#include <stdio.h> /* perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/ioctl.h> /* ioctl */
#include <fscc/fscc.h> /* FSCC_FLUSH_TX */

/*
	This is a simple example showing how to empty all of the data waiting to be
	transmitted out of a port.

*/

int main(void)
{
	int port_fd = 0;

	fprintf(stdout, "WARNING (please read)\n");
	fprintf(stdout, "--------------------------------------------------\n");
	fprintf(stdout, "This limited example is for illustrative use only.\n" \
			"Do not use this code in a production environment\n" \
			"without adding proper error checking.\n\n");

	port_fd = open("/dev/fscc0", O_WRONLY);

	if (port_fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	ioctl(port_fd, FSCC_FLUSH_TX);

	close(port_fd);

	return EXIT_SUCCESS;
}

