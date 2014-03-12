# Read

###### Support
| Code | Version |
| ---- | ------- |
| fscc-linux | 2.0.0 |


## Read
### Function
The Linux [`read`](http://linux.die.net/man/3/read) is used to read data from the port.

| Return Value | Value | Cause |
| ------------ | -----:| ----- |
| `EOPNOTSUPP` | 95 (0x5F) | Using the synchronous port while in asynchronous mode |
| `ENOBUFS` | 105 (0x69) | The buffer size is smaller than the next frame |

###### Examples
```c
#include <unistd.h>
...

char idata[20] = {0};
unsigned bytes_read;

bytes_read = read(fd, idata, sizeof(idata));
```

### Command Line
###### Examples
```
cat /dev/fscc0
```


### Additional Resources
- Complete example: [`examples/tutorial.c`](../examples/tutorial.c)
