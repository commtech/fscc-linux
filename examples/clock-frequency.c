#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* close */
#include <fscc.h> /* FSCC_* */

int main(void)
{
    int fd = 0;
    char clock_bits[20] = {0xff, 0xff, 0xff, 0x01, 0x84, 0x01, 0x48, 0x72,
                           0x9a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x04, 0xe0, 0x01};

    fd = open("/dev/fscc0", O_RDWR);

    ioctl(fd, FSCC_SET_CLOCK_BITS, &clock_bits);

    close(fd);

    return 0;
}
