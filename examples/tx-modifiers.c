#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* close */
#include <fscc.h> /* FSCC_* */

/*
    This is a simple example showing how to change the transmit type for each
    port.

    Valid TX_MODIFIERS are:
    ---------------------------------------------------------------------------
    XF - Normal transmit - disable modifiers
    XREP - Transmit repeat
    TXT - Transmit on timer
    TXEXT - Transmit on external signal

*/

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

