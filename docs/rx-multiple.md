# RX Multiple

###### Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.2.4` 


## Get
### IOCTL
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

### Sysfs
```
/sys/class/fscc/fscc*/settings/rx_multiple
```

###### Examples
```
cat /sys/class/fscc/fscc0/settings/rx_multiple
```


## Enable
### IOCTL
```c
FSCC_ENABLE_RX_MULTIPLE
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_ENABLE_RX_MULTIPLE);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/rx_multiple
```

###### Examples
```
echo 1 > /sys/class/fscc/fscc0/settings/rx_multiple
```


## Disable
### IOCTL
```c
FSCC_DISABLE_RX_MULTIPLE
```

###### Examples
```c
#include <fscc.h>
...

ioctl(fd, FSCC_DISABLE_RX_MULTIPLE);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/rx_multiple
```

###### Examples
```
echo 0 > /sys/class/fscc/fscc0/settings/rx_multiple
```


### Additional Resources
- Complete example: [`examples\rx-multiple.c`](https://github.com/commtech/fscc-linux/blob/master/examples/rx-multiple.c)
