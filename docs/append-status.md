# Append Status

It is a good idea to pay attention to the status of each frame. For example, you may want to see if the frame's CRC check succeeded or failed.

The FSCC reports this data to you by appending two additional bytes to each frame you read from the card, if you opt-in to see this data. There are a few methods of enabling this additional data.

###### Support
| Code | Version |
| ---- | ------- |
| fscc-linux | 2.0.0 |


## Get
### IOCTL
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

### Sysfs
```
/sys/class/fscc/fscc*/settings/append_status
```

###### Examples
```
cat /sys/class/fscc/fscc0/settings/append_status
```


## Enable
### IOCTL
```c
FSCC_ENABLE_APPEND_STATUS
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_ENABLE_APPEND_STATUS);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/append_status
```

###### Examples
```
echo 1 > /sys/class/fscc/fscc0/settings/append_status
```


## Disable
### IOCTL
```c
FSCC_DISABLE_APPEND_STATUS
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_DISABLE_APPEND_STATUS);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/append_status
```

###### Examples
```
echo 0 > /sys/class/fscc/fscc0/settings/append_status
```


### Additional Resources
- Complete example: [`examples/append-status.c`](../examples/append-status.c)
