# fscc-linux
This README file is best viewed [online](http://github.com/commtech/fscc-linux/).

## Installing Driver

##### Downloading Driver Source Code
The source code for the FSCC driver is hosted on Github code hosting. To check
out the latest code you will need Git and to run the following in a terminal.

```
git clone --recursive git://github.com/commtech/fscc-linux.git fscc
```

_You can also download driver packages directly from our
[website](http://www.commtech-fastcom.com/CommtechSoftware.html).

##### Switch To Stable Version
Now that you have the latest code checked out, switch to the latest stable
version(v2.2.1 is only listed here as an example.

```
git tag
git checkout v2.2.1
```

##### Build Source Code
Run the make command from within the source code directory to build the driver.

```
cd fscc/
make
```

If you would like to enable debug prints within the driver you need to add
the DEBUG option while building the driver.

```
make DEBUG=1
```

Once debugging is enabled you will find extra kernel prints in the
/var/log/messages and /var/log/debug log files.

If the kernel header files you would like to build against are not in the
default location `/lib/modules/$(shell uname -r)/build` then you can specify
the location with the KDIR option while building the driver.

```
make KDIR="/location/to/kernel_headers/"
```

##### Loading Driver
Assuming the driver has been successfully built in the previous step you are
now ready to load the driver so you can begin using it. To do this you insert
the driver's kernel object file (fscc.ko) into the kernel.

```
insmod fscc.ko
```

_You will more than likely need administrator privileges for this and
the following commands._

By default if there are no cards present when the driver is loaded the
insmod will fail with a 'No such device' error. To allow the driver to load
even if no cards are present turn on the 'hot_plug' option.

```
insmod fscc.ko hot_plug=1
```

If no cards are present and the hot_plug option is not enabled (default) you
will see this.

```
insmod fscc.ko hot_plug=0
insmod: error inserting 'fscc.ko': -1 No such device
```

_All driver load time options can be set in your modprobe.conf file for
using upon system boot_


##### Installing Driver
If you would like the driver to load automatically at boot use the included
installer.

```
make install
```

This will also install the header (.h) files and python library and using the
card.

To uninstall, use the included uninstaller.

```
make uninstall
```


## Quick Start Guide
There is documentation for each specific function listed below, but lets get started
with a quick programming example for fun.

_This tutorial has already been set up for you at_ 
[`fscc/examples/tutorial.c`](https://github.com/commtech/fscc-linux/tree/master/examples/tutorial.c).

Create a new C file (named tutorial.c) with the following code.

```c
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fscc.h>

int main(void)
{
    int fd = 0;
    char odata[] = "Hello world!";
    char idata[20];

    /* Open port 0 in a blocking IO mode */
    fd = open("/dev/fscc0", O_RDWR);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    /* Send "Hello world!" text */
    write(fd, odata, sizeof(odata));

    /* Read the data back in (with our loopback connector) */
    read(fd, idata, sizeof(idata));

    fprintf(stdout, "%s\n", idata);

    close(fd);

    return EXIT_SUCCESS;
}
```

For this example I will use the Visual Studio command line compiler, but
you can use your compiler of choice.

```
# gcc -I ..\lib\raw\ tutorial.c
```

Now attach the included loopback connector.

```
# ./a.out
Hello world!
```

You have now transmitted and received an HDLC frame! 


## API Reference

There are likely other configuration options you will need to set up for your 
own program. All of these options are described on their respective documentation page.

- [Connect](https://github.com/commtech/fscc-linux/blob/master/docs/connect.md)
- [Append Status](https://github.com/commtech/fscc-linux/blob/master/docs/append-status.md)
- [Append Timestamp](https://github.com/commtech/fscc-linux/blob/master/docs/append-timestamp.md)
- [Clock Frequency](https://github.com/commtech/fscc-linux/blob/master/docs/clock-frequency.md)
- [Ignore Timeout](https://github.com/commtech/fscc-linux/blob/master/docs/ignore-timeout.md)
- [RX Multiple](https://github.com/commtech/fscc-linux/blob/master/docs/rx-multiple.md)
- [Memory Cap](https://github.com/commtech/fscc-linux/blob/master/docs/memory-cap.md)
- [Purge](https://github.com/commtech/fscc-linux/blob/master/docs/purge.md)
- [Registers](https://github.com/commtech/fscc-linux/blob/master/docs/registers.md)
- [TX Modifiers](https://github.com/commtech/fscc-linux/blob/master/docs/tx-modifiers.md)
- [Write](https://github.com/commtech/fscc-linux/blob/master/docs/write.md)
- [Read](https://github.com/commtech/fscc-linux/blob/master/docs/read.md)
- [Disconnect](https://github.com/commtech/fscc-linux/blob/master/docs/disconnect.md)


There are also multiple code libraries to make development easier.
- [C](https://github.com/commtech/cfscc/)
- [Python](https://github.com/commtech/pyfscc/)


## Asynchronous Communication
The FSCC driver lets the built in Linux serial driver handle asynchronous
communication for our UARTs. The Linux serial driver is highly tested and
likely more stable than anything we could produce in any reasonably amount of
time.

Prior to and after loading the FSCC driver there are a few
steps needed to get the card ready for asynchronous communication.

Some Linux distributions have the default number of serial ports that are
available at boot set to a small number (usually 4). The first four serial
ports are reserved so you will need to change this value to something larger
to be able to configure more serial ports.

There are a couple ways of doing this. The easiest method is by appending
'8250.nr_uarts=x' to your grub boot line. Something like this:

kernel /boot/vmlinuz-2.6.20-15-generic ro quiet splash 8250.nr_uarts=16

This can be done temporarily by pressing 'e' at the grub menu during boot or
by permanently modifying this value which is grub version specific. To do
this please search google for one of the numerous guides on the subject.

Another method is by editing the .config file of you kernel before compiling
it to allow for more serial ports. This is not preferred because you will
need to recompile the kernel for it to take effect. The line you need to
change in the .config file is SERIAL_8250_RUNTIME_UARTS.

Load the FSCC driver. This will handle registering our UARTs with the serial
driver. Our UART's will now appear as ttyS nodes in the
/dev/ directory.

By default the FSCC driver boots up in synchronous communication mode. To
switch to the asynchronous mode you must modify the FSCC card's FCR register
to allow for asynchronous communication. There are multiple ways of doing
this. Possibly the simplest method is using sysfs and the command line.

```
echo 03000000 > /sys/class/fscc/fscc0/registers/fcr
```

More information about using the UART's is available in the 
[SerialFC driver README](https://github.com/commtech/serialfc-linux/blob/master/README.md) file.


### FAQ

##### Migrating From 1.x to 2.x
There are multiple benefits of using the 2.x driver: accurate posix error
codes, intuitive ioctl calls, backend support for multiple languages (Python,
C#) and dynamic memory management are some.

The 1.x driver and the 2.x driver are very similar so porting from one to the
other should be rather painless.

_All ioctl values have changed even if their new names match their old
      names. This means even if you use an ioctl with an identical name, it
      will not work correctly._

Setting register values was split into two different ioctl's in the 1.x
driver, setting all the registers at once and one at a time. In the 2.x
driver these two scenarios have been combined into one ioctl.

Change the following ioctl's to the current ioctl `FSCC_SET_REGISTERS` (see
section V).

`FSCC_WRITE_REGISTER` (setting a single register at a time)
`FSCC_INIT` (setting all registers at a time)

Getting register values was limited to one at a time in the 1.x driver. In
the 2.x driver it has been made more convenient to read multiple register
values.

Change the following ioctl to the current ioctl `FSCC_GET_REGISTERS` (see
section VI).

`FSCC_READ_REGISTER` (reading a single register at a time)

Purging transmit and receive data has not changed. Continue using
`FSCC_PURGE_TX` and `FSCC_PURGE_RX`.

Getting the frame status has now been designed to be configurable. In the
1.x driver you would always have the frame status appended to your data on a
read. In the 2.x driver this can be toggled, and defaults to not appending
the status to the data.

Changing the clock frequency is basically the same but the data structure
and ioctl name are different.

Change the following ioctl to the current ioctl `FSCC_SET_CLOCK_BITS` (see
section VIII).

`FSCC_SET_FREQ` (setting the clock frequency)

In the 1.x driver you passed in a structure composed of both the desired
frequency and the clock bits that represent the frequency. In the 2.x driver
this has been simplified down to just the clock bits.

##### Why are the /dev/fscc* ports not created even though the driver has loaded?
There are a couple of possibilities but you should first check
/var/log/messages for any helpful information. If that doesn't help you
out then continue reading.

One possibility is that there is another driver loaded that has claimed
our cards. For example if your kernel is patched to use our card for
asynchronous transmission the linux serial driver has more than likely
claimed the card and in turn we won't receive the appropriate 'probe'
notifications to load our card.

Another possibility is that you have accidentally tried insmod'ing with
the 'hot_plug' option enabled and your cards are not actually present.
Double check that your card shows up in the output of 'lspci' and make
sure to use hot_plug=0.

##### What does poll() and select() base their information on?
Whether or not you can read data will be based on if there is at least 1
byte of data available to be read in your current mode of operation. For
example, if there is streaming data it will not be considered when in
a frame based mode.

Whether or not you can write data will be based on if you have hit your
output memory cap.

##### Why does executing a purge without a clock put the card in a broken state?
When executing a purge on either the transmitter or receiver there is
a TRES or RRES (command from the CMDR register) happening behind the
scenes. If there is no clock available the command will stall until
a clock is available. This should work in theory but doesn't in
practice. So whenever you need to execute a purge without a clock, first
put it into clock mode 7, execute your purge then return to your other
clock mode.

##### Why am I receiving the error message 'Couldn't register serial port' when loading the driver.
When loading, the driver will attempt to register the board's UARTs with
the built in serial driver. The serial driver statically defines how
many UARTs can be registered and will report this error if there isn't
enough room.

There are multiple ways of allowing more available UART room which can
be found in section VII.


## Dependencies
- Base Installation: >= 2.6.16 (might work with a lower version)
- Sysfs Support: >= 2.6.25
- Other: gcc, make, kernel headers


## API Compatibility
We follow [Semantic Versioning](http://semver.org/) when creating releases.


## License

Copyright (C) 2013 [Commtech, Inc.](http://commtech-fastcom.com)

Licensed under the [GNU General Public License v3](http://www.gnu.org/licenses/gpl.txt).
