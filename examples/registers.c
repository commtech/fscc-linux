#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* close */
#include <fscc.h> /* FSCC_* */

int main(void)
{
    int fd = 0;
    struct fscc_registers regs;

    fd = open("/dev/fscc0", O_RDWR);

    FSCC_REGISTERS_INIT(regs);

    /* Change the CCR0 and BGR elements to our desired values */
    regs.CCR0 = 0x0011201c;
    regs.BGR = 10;

    /* Set the CCR0 and BGR register values */
    ioctl(fd, FSCC_SET_REGISTERS, &regs);

    /* Re-initialize our registers structure */
    FSCC_REGISTERS_INIT(regs);

    /* Mark the CCR0 and CCR1 elements to retrieve values */
    regs.CCR1 = FSCC_UPDATE_VALUE;
    regs.CCR2 = FSCC_UPDATE_VALUE;

    /* Get the CCR1 and CCR2 register values */
    ioctl(fd, FSCC_GET_REGISTERS, &regs);

    close(fd);

    return 0;
}
