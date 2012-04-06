/*
	Copyright (C) 2012 Commtech, Inc.

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

#include <fcntl.h> /* open, O_WRONLY */
#include <unistd.h> /* close */
#include <stdio.h> /* perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/ioctl.h> /* ioctl */
#include <fscc/fscc.h> /* FSCC_SET_CLOCK_BITS */

/*
	This is a simple example showing how to change the clock speed cap for each 
	port.

*/

int main(void)
{
	/* 10 MHz */
	unsigned char clock_bits[20] = {0x01, 0xa0, 0x04, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x9a, 0x4a, 0x41,
                                    0x01, 0x84, 0x01, 0xff, 0xff, 0xff};
	int port_fd = 0;

	fprintf(stdout, "WARNING (please read)\n");
	fprintf(stdout, "--------------------------------------------------\n");
	fprintf(stdout, "This limited example is for illustrative use only.\n" \
			"Do not use this code in a production environment\n" \
			"without adding proper error checking.\n\n");

	port_fd = open("/dev/fscc0", O_WRONLY);

	if (port_fd == -1) {
		perror("open");
		return EXIT_FAILURE;
	}

	/* To calculate the clock bits for your desired frequency call this
	   external function in the include/calculate-clock-bits/ directory.

	   > gcc set-clock-bits.c calculate-clock-bits.c -lm 

	if (calculate_clock_bits(10000000, 10, &clock_bits) != 0) {
		fprintf(stderr, "Error calculating clock bits.\n");
		return EXIT_FAILURE;
	}
	
	printf("Programming word: \n");
	
	for(i = 19; i >= 0; i--) 
		printf("%x:", clock_bits[i]);
		
	printf("\n");
	*/

	ioctl(port_fd, FSCC_SET_CLOCK_BITS, &clock_bits);

	close(port_fd);

	return EXIT_SUCCESS;
}

