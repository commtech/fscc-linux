# TX Modifiers

- XF - Normal transmit - disable modifiers
- XREP - Transmit repeat
- TXT - Transmit on timer
- TXEXT - Transmit on external signal

###### Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.0.0` 

## Get
### IOCTL
```c
FSCC_GET_TX_MODIFIERS
```

###### Examples
```
#include <fscc.h>
...

unsigned modifiers;

ioctl(fd, FSCC_GET_TX_MODIFIERS, &modifiers);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/tx_modifiers
```

###### Examples
```
cat /sys/class/fscc/fscc0/settings/tx_modifiers
```


## Set
### IOCTL
```c
FSCC_SET_TX_MODIFIERS
```

###### Examples
```
#include <fscc.h>
...

ioctl(fd, FSCC_SET_TX_MODIFIERS, XF);
```

### Sysfs
```
/sys/class/fscc/fscc*/settings/tx_modifiers
```

###### Examples
```
echo 0 > /sys/class/fscc/fscc0/settings/tx_modifiers
```


### Additional Resources
- Complete example: [`examples\tx-modifiers.c`](https://github.com/commtech/fscc-linux/blob/master/examples/tx-modifiers.c)
