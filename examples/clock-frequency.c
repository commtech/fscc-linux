#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* close */
#include <fscc.h> /* FSCC_* */
#include "calculate-clock-bits.h"

int main(void)
{
    int fd = 0;
    unsigned char clock_bits[20];

    fd = open("/dev/fscc0", O_RDWR);

    /* 18.432 MHz */
    calculate_clock_bits(18432000, 10, clock_bits);

    ioctl(fd, FSCC_SET_CLOCK_BITS, &clock_bits);

    close(fd);

    return 0;
}
