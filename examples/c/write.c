
#include <fcntl.h> /* open, O_WRONLY */
#include <unistd.h> /* write, close */
#include <stdio.h> /* perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */

int main(void)
{
	ssize_t bytes_written = 0;
	int port_fd = 0;
	char data[] = "Hello world!";

	port_fd = open("/dev/fscc0", O_WRONLY);

	if (port_fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	bytes_written = write(port_fd, data, sizeof(data));
	
	close(port_fd);
	
	return EXIT_SUCCESS;
}

