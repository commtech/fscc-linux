#include <fcntl.h> /* open, O_WRONLY */
#include <unistd.h> /* close */
#include <stdio.h> /* perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/ioctl.h> /* ioctl */
#include <fscc/fscc.h> /* FSCC_FLUSH_RX */

int main(void)
{
	int port_fd = 0;

	port_fd = open("/dev/fscc0", O_WRONLY);

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

	if (close(port_fd) == -1) {
		perror("close");
		return EXIT_FAILURE;
	}	
	
	return EXIT_SUCCESS;
}

