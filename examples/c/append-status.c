#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* read, write, close */
#include <stdio.h> /* fprintf, perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h> /* memset */
#include <sys/ioctl.h> /* ioctl */
#include <fscc/fscc.h> /* FSCC_SET_APPEND_STATUS */

int main(void)
{
	ssize_t bytes_written = 0;
	ssize_t bytes_read = 0;
	int port_fd = 0;
	char data[] = "Hello world!";
	char buffer[20];
	unsigned status = 0;

	port_fd = open("/dev/fscc0", O_RDWR);

	if (port_fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	ioctl(port_fd, FSCC_DISABLE_APPEND_STATUS);
	bytes_written = write(port_fd, data, sizeof(data));

	memset(&buffer, 0, sizeof(buffer));
	bytes_read = read(port_fd, buffer, sizeof(buffer));

	fprintf(stdout, "append_status = off\n");
	fprintf(stdout, "bytes_read = %i\n", bytes_read);
	fprintf(stdout, "data = %s\n", buffer);
	fprintf(stdout, "status = 0x????\n\n");

	ioctl(port_fd, FSCC_ENABLE_APPEND_STATUS);
	bytes_written = write(port_fd, data, sizeof(data));

	memset(&buffer, 0, sizeof(buffer));
	bytes_read = read(port_fd, buffer, sizeof(buffer));
	status = buffer[bytes_read - 2];

	fprintf(stdout, "append_status = on\n");
	fprintf(stdout, "bytes_read = %i\n", bytes_read);
	fprintf(stdout, "data = %s\n", buffer);
	fprintf(stdout, "status = 0x%04x\n", status);

	close(port_fd);

	return EXIT_SUCCESS;
}

/*

OUTPUT
-------------------------------------------------------------------------------

append_status = off
bytes_read = 13
data = Hello world!
status = 0x????

append_status = on
bytes_read = 15
data = Hello world!
status = 0x0004

*/

