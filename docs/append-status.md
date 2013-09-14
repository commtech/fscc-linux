# Append Status

It is a good idea to pay attention to the status of each frame. For example, you
may want to see if the frame's CRC check succeeded or failed.

The FSCC reports this data to you by appending two additional bytes
to each frame you read from the card, if you opt-in to see this data. There are
a few methods of enabling this additional data.

###### Driver Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.0.0` 


## Get
```c
FSCC_GET_APPEND_STATUS
```

###### Examples
```c
#include <fscc.h>
...

unsigned status;

ioctl(fd, FSCC_GET_APPEND_STATUS, &status);
```


## Enable
```c
FSCC_ENABLE_APPEND_STATUS
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_ENABLE_APPEND_STATUS);
```


## Disable
```c
FSCC_DISABLE_APPEND_STATUS
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_DISABLE_APPEND_STATUS);
```


### Additional Resources
- Complete example: [`examples\append-status.c`](https://github.com/commtech/fscc-linux/blob/master/examples/append-status.c)
