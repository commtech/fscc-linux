# Append Timestamp

###### Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.4.0` 


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

ioctl(fd, FSCC_GET_APPEND_TIMESTAMP, &status);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/append_timestamp
```

###### Examples
```
cat /sys/class/fscc/fscc0/settings/append_timestamp
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

ioctl(fd, FSCC_ENABLE_APPEND_TIMESTAMP);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/append_timestamp
```

###### Examples
```
echo 1 > /sys/class/fscc/fscc0/settings/append_timestamp
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

ioctl(fd, FSCC_DISABLE_APPEND_TIMESTAMP);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/append_timestamp
```

###### Examples
```
echo 0 > /sys/class/fscc/fscc0/settings/append_timestamp
```


### Additional Resources
- Complete example: [`examples\append-timestamp.c`](https://github.com/commtech/fscc-linux/blob/master/examples/append-timestamp.c)
