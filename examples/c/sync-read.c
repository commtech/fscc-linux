/*
	Copyright (C) 2010  Commtech, Inc.

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
	along with fscc-linux.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <fcntl.h> /* open, O_RDONLY */
#include <unistd.h> /* read, close */
#include <stdio.h> /* fprintf, perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h> /* memset */

int main(void)
{
	ssize_t bytes_read = 0;
	int port_fd = 0;
	char data[20];

	port_fd = open("/dev/fscc0", O_RDONLY);

	if (port_fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	memset(&data, 0, sizeof(data));

	bytes_read = read(port_fd, data, sizeof(data));

	if (bytes_read == -1) {
		perror("read");

		close(port_fd);

		return EXIT_FAILURE;
	}

	fprintf(stdout, "%s\n", data);

	close(port_fd);

	return EXIT_SUCCESS;
}

