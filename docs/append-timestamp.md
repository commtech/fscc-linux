# Append Timestamp

###### Driver Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.4.0` 


## Get
```c
FSCC_GET_APPEND_STATUS
```

###### Examples
```c
#include <fscc.h>
...

unsigned status;

ioctl(fd, FSCC_GET_APPEND_TIMESTAMP, &status);
```


## Enable
```c
FSCC_ENABLE_APPEND_STATUS
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_ENABLE_APPEND_TIMESTAMP);
```


## Disable
```c
FSCC_DISABLE_APPEND_STATUS
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_DISABLE_APPEND_TIMESTAMP);
```


### Additional Resources
- Complete example: [`examples\append-timestamp.c`](https://github.com/commtech/fscc-linux/blob/master/examples/append-timestamp.c)
