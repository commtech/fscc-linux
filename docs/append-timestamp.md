# Append Timestamp

[`do_gettimeofday`](http://www.fsl.cs.sunysb.edu/kernel-api/re29.html) is used to acquire the timestamp upon complete reception of a frame.

_We will be moving to [`getnstimeofday`](http://www.gnugeneration.com/books/linux/2.6.20/kernel-api/re32.html) in the 3.0 driver series._


###### Support
| Code | Version |
| ---- | ------- |
| fscc-linux | 2.4.0 |


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
- Complete example: [`examples/append-timestamp.c`](../examples/append-timestamp.c)
