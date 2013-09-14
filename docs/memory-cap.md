# Memory Cap
If your system has limited memory available, there are safety checks in place to 
prevent spurious incoming data from overrunning your system. Each port has an 
option for setting it's input and output memory cap.


###### Driver Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.0.0` 


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

| Parameter   | Type                       | Description
| ----------- | -------------------------- | --------------------------------------
| `memcap`    | `struct fscc_memory_cap *` | The memory cap structure to initialize

The `FSCC_MEMORY_CAP_INIT` macro should be called each time you use the 
`struct fscc_memory_cap` structure. An initialized structure will allow you to 
only set/receive the memory cap you need.


## Get
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

At this point `memcap.input` and `memcap.output` would be set to their respective
values.


## Set
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


### Additional Resources
- Complete example: [`examples\memory-cap.c`](https://github.com/commtech/fscc-linux/blob/master/examples/memory-cap.c)
