#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* close */
#include <fscc.h> /* FSCC_* */

int main(void)
{
    int fd = 0;
    unsigned status = 0;

    fd = open("/dev/fscc0", O_RDWR);

    ioctl(fd, FSCC_GET_APPEND_STATUS, &status);

    ioctl(fd, FSCC_DISABLE_APPEND_STATUS);
    ioctl(fd, FSCC_ENABLE_APPEND_STATUS);

    close(fd);

    return 0;
}
