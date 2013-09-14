# Write


###### Driver Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.0.0` 


## Write
The Linux [`write`](http://linux.die.net/man/3/write)
is used to write data to the port.

###### Examples
```c
#include <unistd.h>
#include <fscc.h>
...

char odata[] = "Hello world!";
unsigned bytes_written;

bytes_read = write(fd, odata, sizeof(odata));
```


### Additional Resources
- Complete example: [`examples\tutorial.c`](https://github.com/commtech/fscc-linux/blob/master/examples/tutorial.c)
