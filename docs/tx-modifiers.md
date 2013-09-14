# TX Modifiers

- XF - Normal transmit - disable modifiers
- XREP - Transmit repeat
- TXT - Transmit on timer
- TXEXT - Transmit on external signal

###### Driver Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.0.0` 

## Get
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


## Set
```c
FSCC_SET_TX_MODIFIERS
```

###### Examples
```
#include <fscc.h>
...

ioctl(fd, FSCC_SET_TX_MODIFIERS, &modifiers);
```


### Additional Resources
- Complete example: [`examples\tx-modifiers.c`](https://github.com/commtech/fscc-linux/blob/master/examples/tx-modifiers.c)
