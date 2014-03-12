# Purge

Between the hardware FIFO and the driver's software buffers there are multiple places data could be stored, excluding your application code. If you ever need to clear this data and start fresh, there are a couple of methods you can use.

An `FSCC_PURGE_RX` is required after changing the `MODE` bits in the `CCR0` register. If you need to change the `MODE` bits but don't have a clock present, change the `CM` bits to `0x7` temporarily. This will give you an internal clock to switch modes. You can then switch to your desired `CM` now that your `MODE` is locked in.

###### Support
| Code | Version |
| ---- | ------- |
| fscc-linux | 2.0.0 |


## Execute
### IOCTL
```c
FSCC_PURGE_TX
FSCC_PURGE_RX
```

| Return Value | Value | Cause |
| ------------ | -----:| ----- |
| `ETIMEDOUT` | 110 (0x6E) | Command timed out (missing clock) |

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
- Complete example: [`examples/purge.c`](../examples/purge.c)
