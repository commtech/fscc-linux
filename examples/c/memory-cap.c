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
#include <sys/ioctl.h> /* ioctl */
#include <fscc/fscc.h> /* FSCC_MEMORY_CAP_INIT, FSCC_SET_MEMORY_CAP */

/*
	This is a simple example showing how to change the memory cap for each 
	port.

*/

int main(void)
{
	struct fscc_memory_cap memory_cap;
	int port_fd = 0;

	fprintf(stdout, "WARNING (please read)\n");
	fprintf(stdout, "--------------------------------------------------\n");
	fprintf(stdout, "This limited example is for illustrative use only.\n" \
			"Do not use this code in a production environment\n" \
			"without adding proper error checking.\n\n");

	/* Initializes memory cap values to -1 which means do nothing. */
	FSCC_MEMORY_CAP_INIT(memory_cap);

	port_fd = open("/dev/fscc0", O_RDWR);

	if (port_fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	memory_cap.input = 1000000;

	ioctl(port_fd, FSCC_SET_MEMORY_CAP, &memory_cap);

	close(port_fd);

	return EXIT_SUCCESS;
}
