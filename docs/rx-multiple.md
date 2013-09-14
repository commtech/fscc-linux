# RX Multiple

###### Driver Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.2.4` 


## Get
```c
FSCC_GET_RX_MULTIPLE
```

###### Examples
```c
#include <fscc.h>
...

unsigned status;

ioctl(fd, FSCC_GET_RX_MULTIPLE, &status);
```


## Enable
```c
FSCC_ENABLE_RX_MULTIPLE
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_ENABLE_RX_MULTIPLE);
```

###### Support
| Code           | Version
| -------------- | --------
| `cfscc`        | `v1.0.0`


## Disable
```c
FSCC_DISABLE_RX_MULTIPLE
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_DISABLE_RX_MULTIPLE);
```

###### Support
| Code           | Version
| -------------- | --------
| `cfscc`        | `v1.0.0`


### Additional Resources
- Complete example: [`examples\rx-multiple.c`](https://github.com/commtech/fscc-linux/blob/master/examples/rx-multiple.c)
