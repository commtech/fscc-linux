# Ignore Timeout

###### Support
| Code | Version |
| ---- | ------- |
| fscc-linux | 2.0.0 |


## Get
### IOCTL
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

### Sysfs
```
/sys/class/fscc/fscc*/settings/ignore_timeout
```

###### Examples
```
cat /sys/class/fscc/fscc0/settings/ignore_timeout
```


## Enable
### IOCTL
```c
FSCC_ENABLE_IGNORE_TIMEOUT
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_ENABLE_IGNORE_TIMEOUT);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/ignore_timeout
```

###### Examples
```
echo 1 > /sys/class/fscc/fscc0/settings/ignore_timeout
```


## Disable
### IOCTL
```c
FSCC_DISABLE_IGNORE_TIMEOUT
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_DISABLE_IGNORE_TIMEOUT);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/ignore_timeout
```

###### Examples
```
echo 0 > /sys/class/fscc/fscc0/settings/ignore_timeout
```


### Additional Resources
- Complete example: [`examples/ignore-timeout.c`](../examples/ignore-timeout.c)
