#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* close */
#include <fscc.h> /* FSCC_* */

int main(void)
{
    int fd = 0;
    unsigned modifiers;

    fd = open("/dev/fscc0", O_RDWR);

    ioctl(fd, FSCC_GET_TX_MODIFIERS, &modifiers);

    /* Enable transmit repeat & transmit on timer */
    ioctl(fd, FSCC_SET_TX_MODIFIERS, TXT | XREP);

    /* Revert to normal transmission */
    ioctl(fd, FSCC_SET_TX_MODIFIERS, XF);

    close(fd);

    return 0;
}

