#include <fcntl.h> /* open, O_WRONLY */
#include <unistd.h> /* close */
#include <stdio.h> /* perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/ioctl.h> /* ioctl */
#include <fscc/fscc.h> /* FSCC_SET_CLOCK_BITS */

int main(void)
{
	int port_fd = 0;
	char clock_bits[20] = {0xff, 0xff, 0xff, 0x01, 0x84, 0x01, 0x48, 0x72,
	                       0x9a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                       0x00, 0x04, 0xe0, 0x01};

	port_fd = open("/dev/fscc0", O_WRONLY);

	if (port_fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	ioctl(port_fd, FSCC_SET_CLOCK_BITS, &clock_bits);

	close(port_fd);
	
	return EXIT_SUCCESS;
}

