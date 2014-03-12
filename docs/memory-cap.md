# Memory Cap

If your system has limited memory available, there are safety checks in place to prevent spurious incoming data from overrunning your system. Each port has an option for setting it's input and output memory cap.


###### Support
| Code | Version |
| ---- | ------- |
| fscc-linux | 2.0.0 |


## Structure
```c
struct fscc_memory_cap {
    int input;
    int output;
};
```


## Macros
```c
FSCC_MEMORY_CAP_INIT(memcap)
```

| Parameter | Type | Description |
| --------- | ---- | ----------- |
| `memcap` | `struct fscc_memory_cap *` | The memory cap structure to initialize |

The `FSCC_MEMORY_CAP_INIT` macro should be called each time you use the `struct fscc_memory_cap` structure. An initialized structure will allow you to only set/receive the memory cap you need.


## Get
### IOCTL
```c
FSCC_GET_MEMORY_CAP
```

###### Examples
```
#include <fscc.h>
...

struct fscc_memory_cap memcap;

FSCC_MEMORY_CAP_INIT(memcap);

ioctl(fd, FSCC_GET_MEMORY_CAP, &memcap);
```

At this point `memcap.input` and `memcap.output` would be set to their respective values.

### Sysfs
```
/sys/class/fscc/fscc*/settings/input_memory_cap
/sys/class/fscc/fscc*/settings/output_memory_cap
```

###### Examples
```
cat /sys/class/fscc/fscc0/settings/input_memory_cap
cat /sys/class/fscc/fscc0/settings/output_memory_cap
```


## Set
### IOCTL
```c
FSCC_SET_MEMORY_CAP
```

###### Examples
```
#include <fscc.h>
...

struct fscc_memory_cap memcap;

FSCC_MEMORY_CAP_INIT(memcap);

memcap.input = 1000000; /* 1 MB */
memcap.output = 2000000; /* 2 MB */

ioctl(fd, FSCC_SET_MEMORY_CAP, &memcap);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/input_memory_cap
/sys/class/fscc/fscc*/settings/output_memory_cap
```

###### Examples
```
echo 1000000 > /sys/class/fscc/fscc0/settings/input_memory_cap
echo 2000000 > /sys/class/fscc/fscc0/settings/output_memory_cap
```


### Additional Resources
- Complete example: [`examples/memory-cap.c`](../examples/memory-cap.c)
