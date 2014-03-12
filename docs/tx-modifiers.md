# TX Modifiers

| Modifiers | Value | Description |
| --------- | -----:| ----------- |
| `XF` | 0 | Normal transmit (disable modifiers) |
| `XREP` | 1 | Transmit frame repeatedly |
| `TXT` | 2 | Transmit frame on timer |
| `TXEXT` | 4 | Transmit frame on external signal |

###### Support
| Code | Version |
| ---- | ------- |
| fscc-linux | 2.0.0 |

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
- Complete example: [`examples/tx-modifiers.c`](../examples/tx-modifiers.c)
