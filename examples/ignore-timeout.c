#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* close */
#include <fscc.h> /* FSCC_* */

int main(void)
{
    int fd = 0;
    unsigned status = 0;

    fd = open("/dev/fscc0", O_RDWR);

    ioctl(fd, FSCC_GET_IGNORE_TIMEOUT, &status);

    ioctl(fd, FSCC_ENABLE_IGNORE_TIMEOUT);
    ioctl(fd, FSCC_DISABLE_IGNORE_TIMEOUT);

    close(fd);

    return 0;
}
