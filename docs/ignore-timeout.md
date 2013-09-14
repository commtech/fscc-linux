# Ignore Timeout

###### Driver Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.0.0` 


## Get
```c
FSCC_GET_IGNORE_TIMEOUT
```

###### Examples
```c
#include <fscc.h>
...

unsigned status;

ioctl(fd, FSCC_GET_IGNORE_TIMEOUT, &status);
```


## Enable
```c
FSCC_ENABLE_IGNORE_TIMEOUT
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_ENABLE_IGNORE_TIMEOUT);
```


## Disable
```c
FSCC_DISABLE_IGNORE_TIMEOUT
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_DISABLE_IGNORE_TIMEOUT);
```


### Additional Resources
- Complete example: [`examples\ignore-timeout.c`](https://github.com/commtech/fscc-linux/blob/master/examples/ignore-timeout.c)
