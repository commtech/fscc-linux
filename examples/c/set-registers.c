#include <fcntl.h> /* open, O_WRONLY */
#include <unistd.h> /* close */
#include <stdio.h> /* perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/ioctl.h> /* ioctl */
#include <fscc/fscc.h> /* FSCC_REGISTERS_INIT, FSCC_SET_REGISTERS */

int main(void)
{
	struct fscc_registers regs;
	int port_fd = 0;

	port_fd = open("/dev/fscc0", O_WRONLY);

	if (port_fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	FSCC_REGISTERS_INIT(regs);

	regs.CCR0 = 0x0011201c;
	regs.CCR1 = 0x00000018;
	regs.CCR2 = 0x00000000;

	ioctl(port_fd, FSCC_SET_REGISTERS, &regs);

	close(port_fd);

	return EXIT_SUCCESS;
}

