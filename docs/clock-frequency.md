# Clock Frequency

The FSCC device has a programmable clock that can be set anywhere from 20 KHz to 200 MHz. However, this is not the full operational range of an FSCC port, only the range that the on-board clock can be set to.

Using one of the synchronous modes you can only receive data consistently up to 30 MHz (when you are using an external clock). If you are transmitting data using an internal clock, you can safely receive data consistently up to 50 MHz.

Lower clock rates (less than 1 MHz for example) can take a long time for the frequency generator to finish. If you run into this situation we recommend using a larger frequency and then dividing it down to your desired baud rate using the `BGR` register.

_If you are receiving timeout errors when using slow data rates you can bypass the safety checks by using the [`FSCC_ENABLE_IGNORE_TIMEOUT`](https://github.com/commtech/fscc-linux/blob/master/docs/ignore-timeout.md) option._

###### Support
| Code | Version |
| ---- | ------- |
| fscc-linux | 2.0.0 |


## Set
```c
FSCC_SET_CLOCK_BITS
```

###### Examples
Set the port's clock frequency to 10 MHz.
```c
#include <fscc.h>
...

/* 10 MHz */
unsigned char clock_bits[20] = {0x01, 0xa0, 0x04, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x9a, 0x4a, 0x41,
                               0x01, 0x84, 0x01, 0xff, 0xff, 0xff};

ioctl(fd, FSCC_SET_CLOCK_BITS, &clock_bits);
```


### Additional Resources
- Complete example: [`examples/clock-frequency.c`](../examples/clock-frequency.c)
