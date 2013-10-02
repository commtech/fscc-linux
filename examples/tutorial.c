#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int fd = 0;
    char odata[] = "Hello world!";
    char idata[20];

    /* Open port 0 */
    fd = open("/dev/fscc0", O_RDWR);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    /* Send "Hello world!" text */
    write(fd, odata, sizeof(odata));

    /* Read the data back in (with our loopback connector) */
    read(fd, idata, sizeof(idata));

    fprintf(stdout, "%s\n", idata);

    close(fd);

    return EXIT_SUCCESS;
}
