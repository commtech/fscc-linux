import struct
import fcntl
import io

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
        io.FileIO.__init__(self, file, mode)
        self.reset_registers()

        if append_status:
            self.enable_append_status()
        else:
            self.disable_append_status()

    def reset_registers(self):
        for register in self.register_names:
            setattr(self, register, -1)

    def get_registers(self):
        regs = self._construct_full_registers_list()

        buf = fcntl.ioctl(self, FSCC_GET_REGISTERS,
                             struct.pack("i" * len(regs), *regs))

        regs = struct.unpack("i" * len(regs), buf)

        for i in range(len(regs)):
            if regs[i] != -1:
                self._set_register_by_index(i, regs[i])

    def set_registers(self):
        regs = self._construct_full_registers_list()

        fcntl.ioctl(self, FSCC_SET_REGISTERS,
                    struct.pack("i" * len(regs), *regs))

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
                      "VSTR", "IMR", "FCR"]

    def _construct_full_registers_list(self):
        return [-1, -1, self.FIFOT, -1, -1, self.CMDR, self.STAR, self.CCR0,
                self.CCR1, self.CCR2, self.BGR, self.SSR, self.SMR, self.TSR,
                self.TMR, self.RAR, self.RAMR, self.PPR, self.TCR, self.VSTR,
                -1, self.IMR, self.FCR]

    def _set_register_by_index(self, index, value):
        if index == 2:
            self.FIFOT = value
            return

        if index == 5:
            self.CMDR = value
            return

        if index == 6:
            self.STAR = value
            return

        if index == 7:
            self.CCR0 = value
            return

        if index == 8:
            self.CCR1 = value
            return

        if index == 9:
            self.CCR2 = value
            return

        if index == 10:
            self.BGR = value
            return

        if index == 11:
            self.SSR = value
            return

        if index == 12:
            self.SMR = value
            return

        if index == 13:
            self.TSR = value
            return

        if index == 14:
            self.TMR = value
            return

        if index == 15:
            self.RAR = value
            return

        if index == 16:
            self.RAMR = value
            return

        if index == 17:
            self.PPR = value
            return

        if index == 18:
            self.TCR = value
            return

        if index == 19:
            self.VSTR = value
            return

        if index == 21:
            self.IMR = value
            return

        if index == 22:
            self.FCR = value
            return
