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

#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* read, write, close */
#include <stdio.h> /* perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h> /* memset */
#include <sys/ioctl.h> /* ioctl */
#include <fscc/fscc.h> /* FSCC_FLUSH_RX, FSCC_FLUSH_TX */

int main(void)
{
	ssize_t bytes_written = 0;
	ssize_t bytes_read = 0;
	int port_fd = 0;
	char data[] = "Hello world!";
	char buffer[20];

	port_fd = open("/dev/fscc0", O_RDWR);

	if (port_fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	if (ioctl(port_fd, FSCC_FLUSH_RX) == -1) {
		perror("FSCC_FLUSH_RX");

		if (close(port_fd) == -1)
			perror("close");

		return EXIT_FAILURE;
	}

	if (ioctl(port_fd, FSCC_FLUSH_TX) == -1) {
		perror("FSCC_FLUSH_TX");

		if (close(port_fd) == -1)
			perror("close");

		return EXIT_FAILURE;
	}

	bytes_written = write(port_fd, data, sizeof(data));

	if (bytes_written < 0) {
		perror("write");

		if (close(port_fd) == -1)
			perror("close");

		return EXIT_FAILURE;
	}

	memset(&data, 0, sizeof(buffer));

	bytes_read = read(port_fd, buffer, sizeof(buffer));

	if (bytes_read == -1) {
		perror("read");

		if (close(port_fd) == -1)
			perror("close");

		return EXIT_FAILURE;
	}

	if (close(port_fd) == -1) {
		perror("close");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

