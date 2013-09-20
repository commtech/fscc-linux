# Purge
Between the hardware FIFO and the driver's software buffers there are multiple places data could 
be stored, excluding your application code. If you ever need to clear this data and start fresh, 
there are a couple of methods you can use.

###### Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.0.0` 


## Execute
### IOCTL
```c
FSCC_PURGE_TX
FSCC_PURGE_RX
```

| Return Value | Cause
| ------------ | ------------------------------------------------------------------
| `-ETIMEDOUT` | You are executing a command that requires a transmit clock present

###### Examples
Purge the transmit data.
```c
#include <fscc.h>
...

ioctl(fd, FSCC_PURGE_TX);
```

Purge the receive data.
```c
#include <fscc.h>
...

ioctl(fd, FSCC_PURGE_RX);
```

### Sysfs
```
/sys/class/fscc/fscc*/commands/purge_tx
/sys/class/fscc/fscc*/commands/purge_rx
```

###### Examples
```
echo 1 > /sys/class/fscc/fscc*/commands/purge_tx
echo 1 > /sys/class/fscc/fscc*/commands/purge_rx
```


### Additional Resources
- Complete example: [`examples\purge.c`](https://github.com/commtech/fscc-linux/blob/master/examples/purge.c)
