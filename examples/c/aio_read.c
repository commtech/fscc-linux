#include <fcntl.h> /* open, O_RDONLY */
#include <unistd.h> /* close */
#include <stdio.h> /* fprintf, perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h> /* memset */
#include <aio.h> /* aio_read, aio_error, aio_return */
#include <errno.h> /* EINPROGRESS */

/*
   NOTE: You may need to link against the retail time library to use the aio_*
         functions e.g. gcc -lrt aio_read.c

*/

int main(void)
{
	ssize_t bytes_read = 0;
	int port_fd = 0;
	char data[20];
	struct aiocb aio;

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

