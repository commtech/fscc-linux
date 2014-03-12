# Write

###### Support
| Code | Version |
| ---- | ------- |
| fscc-linux | 2.0.0 |


## Write
The Linux [`write`](http://linux.die.net/man/3/write) is used to write data to the port.

| Return Value | Value | Cause |
| ------------ | -----:| ----- |
| `EOPNOTSUPP` | 95 (0x5F) | Using the synchronous port while in asynchronous mode |
| `ENOBUFS` | 105 (0x69) | The write size exceeds the output memory usage cap |
| `ETIMEDOUT` | 110 (0x6E) | Command timed out (missing clock) |

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
- Complete example: [`examples/tutorial.c`](../examples/tutorial.c)
