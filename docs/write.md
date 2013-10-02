# Write


###### Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.0.0`


## Write
The Linux [`write`](http://linux.die.net/man/3/write)
is used to write data to the port.

| Return Value | Cause
| ------------ | ------------------------------------------------------------------
| `-EOPNOTSUPP` | You are using a synchronous handle while in asynchronous mode (use /dev/ttyS node)
| `-ETIMEDOUT`  | There isn't a transmit clock present (check disabled using 'ignore_timeout' command line parameter)
| `-ENOBUFS`    | Transmitting the frame will excede your output memory cap

###### Examples
### Function
```c
#include <unistd.h>
...

char odata[] = "Hello world!";
unsigned bytes_written;

bytes_read = write(fd, odata, sizeof(odata));
```

### Command Line
###### Examples
```
echo "Hello world!" > /dev/fscc0
```


### Additional Resources
- Complete example: [`examples\tutorial.c`](https://github.com/commtech/fscc-linux/blob/master/examples/tutorial.c)
