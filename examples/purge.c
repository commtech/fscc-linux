#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* close */
#include <fscc.h> /* FSCC_* */

int main(void)
{
    int fd = 0;

    fd = open("/dev/fscc0", O_RDWR);

    ioctl(fd, FSCC_PURGE_TX);
    ioctl(fd, FSCC_PURGE_RX);

    close(fd);

    return 0;
}
