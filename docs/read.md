# Read
The length argument of the `fscc_read()` function means different things depending
on the mode you are using.

In a frame based mode the length argument specifies the maximum frame size
to return. If the next queued frame is larger than the size you specified
the error `FSCC_BUFFER_TOO_SMALL` is returned and the data will remain 
waiting for a `fscc_read()` of a larger value. If a `fscc_read()` length is specified 
that is larger than the length of multiple frames in queue, you will still only receive 
one frame per `fscc_read()` call.

In streaming mode (no frame termination) the length argument specifies the
maximum amount of data to return. If there are 100 bytes of streaming data
in the card and you `fscc_read()` with a length of 50, you will receive 50 bytes.
If you do a `fscc_read()` of 200 bytes, you will receive the 100 bytes available.

Frame based data and streaming data are kept separate within the driver.
To understand what this means, first imagine the following scenario. You are in a
frame based mode and receive a couple of frames. You then switch to
streaming mode and receive a stream of data. When calling `fscc_read()`
you will receive the the streaming data until you switch back into a frame based
mode and do a `fscc_read()`.

Most users will want the advanced I/O capabilities included by using the Windows
[OVERLAPPED I/O API](http://msdn.microsoft.com/en-us/library/windows/desktop/ms686358.aspx). 
We won't duplicate the documentation here, but for your reference, here is an [article]
(http://blogs.msdn.com/b/oldnewthing/archive/2011/02/02/10123392.aspx) on a common
bug developers introduce while trying to cancel I/O operations when using OVERLAPPED I/O.

###### Support
| Code         | Version
| ------------ | --------
| `fscc-linux` | `v2.0.0` 


## Read
### Function
The Linux [`read`](http://linux.die.net/man/3/read)
is used to read data from the port.

| Return Value | Cause
| ------------ | ------------------------------------------------------------------
| `-ENOBUFS`    | Your buffer is smaller than the next frame (in a frame based mode)
| `-EOPNOTSUPP` | You are using a synchronous handle while in asynchronous mode (use /dev/ttyS node)

###### Examples
```c
#include <unistd.h>
...

char idata[20] = {0};
unsigned bytes_read;

bytes_read = read(fd, idata, sizeof(idata));
```

### Command Line
###### Examples
```
cat /dev/fscc0
```


### Additional Resources
- Complete example: [`examples\tutorial.c`](https://github.com/commtech/fscc-linux/blob/master/examples/tutorial.c)
