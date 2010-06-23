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

