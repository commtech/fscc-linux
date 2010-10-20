import struct
import fcntl
import io
import os
import errno

IOCPARM_MASK = 0x7f
#IOC_NONE = 0x20000000
IOC_NONE = 0x00000000
IOC_WRITE = 0x40000000
IOC_READ = 0x80000000


def FIX(x):
    return struct.unpack("i", struct.pack("I", x))[0]


def _IO(x, y):
    return FIX(IOC_NONE | (x << 8) | y)


def _IOR(x, y, t):
    return FIX(IOC_READ | ((t & IOCPARM_MASK) << 16) | (x << 8) | y)


def _IOW(x, y, t):
    return FIX(IOC_WRITE | ((t & IOCPARM_MASK) << 16) | (x << 8) | y)


def _IOWR(x, y, t):
    return FIX(IOC_READ | IOC_WRITE | ((t & IOCPARM_MASK) << 16) | (x << 8) | y)

FSCC_IOCTL_MAGIC = 0x18

FSCC_GET_REGISTERS = _IOR(FSCC_IOCTL_MAGIC, 0, struct.calcsize("i"))
FSCC_SET_REGISTERS = _IOW(FSCC_IOCTL_MAGIC, 1, struct.calcsize("i"))
FSCC_FLUSH_TX = _IO(FSCC_IOCTL_MAGIC, 2)
FSCC_FLUSH_RX = _IO(FSCC_IOCTL_MAGIC, 3)
FSCC_ENABLE_APPEND_STATUS = _IO(FSCC_IOCTL_MAGIC, 4)
FSCC_DISABLE_APPEND_STATUS = _IO(FSCC_IOCTL_MAGIC, 5)

FSCC_UPDATE_VALUE = -2


class Port(io.FileIO):

    def __init__(self, file, mode, append_status=False):
        if not os.path.exists(file):
            raise IOError(errno.ENOENT, os.strerror(errno.ENOENT), file)

        io.FileIO.__init__(self, file, mode)
        self.clear_registers()

        if append_status:
            self.enable_append_status()
        else:
            self.disable_append_status()

    def clear_registers(self):
        for register in self.register_names:
            setattr(self, register, -1)

    def get_registers(self):
        registers = self._construct_full_registers_list()

        buf = fcntl.ioctl(self, FSCC_GET_REGISTERS,
                          struct.pack("q" * len(registers), *registers))

        regs = struct.unpack("q" * len(registers), buf)

        for i, register in enumerate(registers):
            if register != -1:
                self._set_register_by_index(i, regs[i])

    def set_registers(self):
        regs = self._construct_full_registers_list()

        fcntl.ioctl(self, FSCC_SET_REGISTERS,
                    struct.pack("q" * len(regs), *regs))

    def flush_tx(self):
        fcntl.ioctl(self, FSCC_FLUSH_TX)

    def flush_rx(self):
        fcntl.ioctl(self, FSCC_FLUSH_RX)

    def enable_append_status(self):
        fcntl.ioctl(self, FSCC_ENABLE_APPEND_STATUS)

    def disable_append_status(self):
        fcntl.ioctl(self, FSCC_DISABLE_APPEND_STATUS)

    def read(self, num_bytes):
        if num_bytes:
            return super(io.FileIO, self).read(num_bytes)

    register_names = ["FIFOT", "CMDR", "STAR", "CCR0", "CCR1", "CCR2", "BGR",
                      "SSR", "SMR", "TSR", "TMR", "RAR", "RAMR", "PPR", "TCR",
                      "VSTR", "IMR", "DPLLR", "FCR"]

    def _construct_full_registers_list(self):
        return [-1, -1, self.FIFOT, -1, -1, self.CMDR, self.STAR, self.CCR0,
                self.CCR1, self.CCR2, self.BGR, self.SSR, self.SMR, self.TSR,
                self.TMR, self.RAR, self.RAMR, self.PPR, self.TCR, self.VSTR,
                -1, self.IMR, self.DPLLR, self.FCR]

    def _set_register_by_index(self, index, value):
        data = [("FIFOT", 2), ("CMDR", 5), ("STAR", 6), ("CCR0", 7),
                ("CCR1", 8), ("CCR2", 9), ("BGR", 10), ("SSR", 11),
                ("SMR", 12), ("TSR", 13), ("TMR", 14), ("RAR", 15),
                ("RAMR", 16), ("PPR", 17), ("TCR", 18), ("VSTR", 19),
                ("IMR", 21), ("DPLLR", 22), ("FCR", 23)]

        for r, i in data:
            if i == index:
                setattr(self, r, value)
