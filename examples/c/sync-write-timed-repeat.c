/*
    Copyright (C) 2013 Commtech, Inc.

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
    along with fscc-linux. If not, see <http://www.gnu.org/licenses/>.

*/

#include <fcntl.h> /* open, O_WRONLY */
#include <unistd.h> /* write, close */
#include <stdio.h> /* perror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/ioctl.h> /* ioctl */
#include <fscc/fscc.h> /* FSCC_REGISTERS_INIT, FSCC_SET_REGISTERS */


/*
    This is a simple example showing how to write synchronous data to a port.
    It demonstrates the use of two of the transmit moifiers:
    Transmit on Timer (TXT) and Transmit Repeat (XREP)
    This will write "Hello World" to the TxFIFO (once) and will use the onboard
    timer to send "Hello World" once every 10 ms.
*/

int main(void)
{
    ssize_t bytes_written = 0;
    struct fscc_registers regs;
    int port_fd = 0;
    char data[] = "Hello world!";
    /* 4 MHz */
    unsigned char clock_bits[20] = {0x01, 0xe0, 0x04, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x9a, 0x62, 0x48,
                    0x01, 0x84, 0x01, 0xff, 0xff, 0xff};

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
/*
    if (calculate_clock_bits(4000000, 1, &clock_bits) != 0) {
        fprintf(stderr, "Error calculating clock bits.\n");
        return EXIT_FAILURE;
    }
*/
    ioctl(port_fd, FSCC_SET_CLOCK_BITS, &clock_bits);

    FSCC_REGISTERS_INIT(regs);

    regs.CCR0 = 0x0210001e;    //transparent mode, cm=7
    regs.CCR1 = 0x04000008;
    regs.CCR2 = 0x00000000;
    regs.TCR = 0x0004e200; //timer source = osc, cnt=40000
                   //this gets us a transmit every 10ms (100Hz)

    ioctl(port_fd, FSCC_SET_REGISTERS, &regs);

    ioctl(port_fd, FSCC_SET_TX_MODIFIERS, TXT|XREP);

    bytes_written = write(port_fd, data, sizeof(data));

    if (bytes_written < 0) {
        perror("write");

        close(port_fd);

        return EXIT_FAILURE;
    }

    printf("Now put 0x00000001 into the CMDR (from sysfs) to start the timer\n");
    printf("Then put 0x00000002 into the CMDR (from sysfs) to stop the timer\n");

    close(port_fd);

    return EXIT_SUCCESS;
}

