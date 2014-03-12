#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* close */
#include <fscc.h> /* FSCC_* */

int main(void)
{
    int fd = 0;
    struct fscc_memory_cap memcap;

    fd = open("/dev/fscc0", O_RDWR);

    memcap.input = 1000000; /* 1 MB */
    memcap.output = 1000000; /* 1 MB */

    ioctl(fd, FSCC_SET_MEMORY_CAP, &memcap);

    close(fd);

    return 0;
}

