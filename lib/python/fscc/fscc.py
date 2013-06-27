"""
        Copyright (C) 2012 Commtech, Inc.

        This file is part of fscc-linux.

        fscc-linux is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        fscc-linux is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with fscc-linux.  If not, see <http://www.gnu.org/licenses/>.

"""

import struct
import fcntl
import select
import errno
import io
import os

IOCPARM_MASK = 0x7f
IO_NONE = 0x00000000
IO_WRITE = 0x40000000
IO_READ = 0x80000000


def FIX(x):
    return struct.unpack("i", struct.pack("I", x))[0]


def _IO(x, y):
    return FIX(IO_NONE | (x << 8) | y)


def _IOR(x, y, t):
    return FIX(IO_READ | ((t & IOCPARM_MASK) << 16) | (x << 8) | y)


def _IOW(x, y, t):
    return FIX(IO_WRITE | ((t & IOCPARM_MASK) << 16) | (x << 8) | y)


def _IOWR(x, y, t):
    return FIX(IO_READ | IO_WRITE | ((t & IOCPARM_MASK) << 16) | (x << 8) | y)

FSCC_IOCTL_MAGIC = 0x18

FSCC_GET_REGISTERS = _IOR(FSCC_IOCTL_MAGIC, 0, struct.calcsize("P"))
FSCC_SET_REGISTERS = _IOW(FSCC_IOCTL_MAGIC, 1, struct.calcsize("P"))

FSCC_PURGE_TX = _IO(FSCC_IOCTL_MAGIC, 2)
FSCC_PURGE_RX = _IO(FSCC_IOCTL_MAGIC, 3)

FSCC_ENABLE_APPEND_STATUS = _IO(FSCC_IOCTL_MAGIC, 4)
FSCC_DISABLE_APPEND_STATUS = _IO(FSCC_IOCTL_MAGIC, 5)
FSCC_GET_APPEND_STATUS = _IOR(FSCC_IOCTL_MAGIC, 13, struct.calcsize("P"))

FSCC_SET_MEMORY_CAP = _IOW(FSCC_IOCTL_MAGIC, 6, struct.calcsize("P"))
FSCC_GET_MEMORY_CAP = _IOR(FSCC_IOCTL_MAGIC, 7, struct.calcsize("P"))

FSCC_ENABLE_IGNORE_TIMEOUT = _IO(FSCC_IOCTL_MAGIC, 10)
FSCC_DISABLE_IGNORE_TIMEOUT = _IO(FSCC_IOCTL_MAGIC, 11)
FSCC_GET_IGNORE_TIMEOUT = _IOR(FSCC_IOCTL_MAGIC, 15, struct.calcsize("P"))

FSCC_SET_TX_MODIFIERS = _IOW(FSCC_IOCTL_MAGIC, 12, struct.calcsize("i"))
FSCC_GET_TX_MODIFIERS = _IOR(FSCC_IOCTL_MAGIC, 14, struct.calcsize("P"))

FSCC_UPDATE_VALUE = -2

XF, XREP, TXT, TXEXT = 0, 1, 2, 4


class InvalidPortError(Exception):
    """Exception for the situation where a non FSCC port is opened."""
    def __init__(self, file_name):
        self.file_name = file_name

    def __str__(self):
        return "'%s' is not an FSCC port" % self.file_name


class InvalidRegisterError(Exception):
    """Exception for the situation where an invalid register is modified."""
    def __init__(self, register_name):
        self.register_name = register_name

    def __str__(self):
        return "'%s' is an invalid register" % self.register_name


class ReadonlyRegisterError(InvalidRegisterError):
    """Exception for the situation where a read only register is modified."""
    def __str__(self):
        return "'%s' is a readonly register" % self.register_name


class Port(io.FileIO):
    """Commtech FSCC port."""
    class Registers(object):
        """Registers on the FSCC port."""
        register_names = ["FIFOT", "CMDR", "STAR", "CCR0", "CCR1", "CCR2",
                          "BGR", "SSR", "SMR", "TSR", "TMR", "RAR", "RAMR",
                          "PPR", "TCR", "VSTR", "IMR", "DPLLR", "FCR"]

        readonly_register_names = ["STAR", "VSTR"]
        writeonly_register_names = ["CMDR"]

        editable_register_names = [r for r in register_names if r not in
                                   ["STAR", "VSTR"]]

        def __init__(self, port=None):
            self.port = port
            self._clear_registers()

            for register in self.register_names:
                self._add_register(register)

        def __iter__(self):
            registers = [-1, -1, self._FIFOT, -1, -1, self._CMDR, self._STAR,
                         self._CCR0, self._CCR1, self._CCR2, self._BGR,
                         self._SSR, self._SMR, self._TSR, self._TMR, self._RAR,
                         self._RAMR, self._PPR, self._TCR, self._VSTR, -1,
                         self._IMR, self._DPLLR, self._FCR]

            for register in registers:
                yield register

        def _add_register(self, register):
            """Dynamically add a way to edit a register to the port."""
            if register not in self.writeonly_register_names:
                fget = lambda self: self._get_register(register)
            else:
                fget = None

            if register not in self.readonly_register_names:
                fset = lambda self, value: self._set_register(register, value)
            else:
                fset = None

            setattr(self.__class__, register, property(fget, fset, None, ""))

        def _get_register(self, register):
            """Gets the value of a register."""
            if self.port:
                self._clear_registers()
                setattr(self, "_%s" % register, FSCC_UPDATE_VALUE)
                self._get_registers()

            return getattr(self, "_%s" % register)

        def _set_register(self, register, value):
            """Sets the value of a register."""
            if self.port:
                self._clear_registers()

            setattr(self, "_%s" % register, value)

            if self.port:
                self._set_registers()

        def _clear_registers(self):
            """Clears the stored register values."""
            for register in self.register_names:
                setattr(self, "_%s" % register, -1)

        def _get_registers(self):
            """Gets all of the register values."""
            if not self.port:
                return

            registers = list(self)

            buf = fcntl.ioctl(self.port, FSCC_GET_REGISTERS,
                              struct.pack("q" * len(registers), *registers))

            regs = struct.unpack("q" * len(registers), buf)

            for i, register in enumerate(registers):
                if register != -1:
                    self._set_register_by_index(i, regs[i])

        def _set_registers(self):
            """Sets all of the register values."""
            if not self.port:
                return

            registers = list(self)

            fcntl.ioctl(self.port, FSCC_SET_REGISTERS,
                        struct.pack("q" * len(registers), *registers))

        def _set_register_by_index(self, index, value):
            """Sets the value of a register by it's index."""
            data = [("FIFOT", 2), ("CMDR", 5), ("STAR", 6), ("CCR0", 7),
                    ("CCR1", 8), ("CCR2", 9), ("BGR", 10), ("SSR", 11),
                    ("SMR", 12), ("TSR", 13), ("TMR", 14), ("RAR", 15),
                    ("RAMR", 16), ("PPR", 17), ("TCR", 18), ("VSTR", 19),
                    ("IMR", 21), ("DPLLR", 22), ("FCR", 23)]

            for r, i in data:
                if i == index:
                    setattr(self, "_%s" % r, value)

        # Note: clears registers
        def import_from_file(self, import_file):
            """Reads and stores the register values from a file."""
            import_file.seek(0, os.SEEK_SET)

            for line in import_file:
                try:
                    line = str(line, encoding='utf8')
                except:
                    pass

                if line[0] != "#":
                    d = line.split("=")
                    reg_name, reg_val = d[0].strip().upper(), d[1].strip()

                    if reg_name not in self.register_names:
                        raise InvalidRegisterError(reg_name)

                    if reg_name not in self.editable_register_names:
                        raise ReadonlyRegisterError(reg_name)

                    if reg_val[0] == "0" and reg_val[1] in ["x", "X"]:
                        reg_val = int(reg_val, 16)
                    else:
                        reg_val = int(reg_val)

                    setattr(self, reg_name, reg_val)

        def export_to_file(self, export_file):
            """Writes the current register values to a file."""
            for register_name in self.editable_register_names:
                if register_name in self.writeonly_register_names:
                    continue

                value = getattr(self, register_name)

                if value >= 0:
                    export_file.write("%s = 0x%08x\n" % (register_name, value))

    def __init__(self, file_name, mode, append_status=False):
        if not os.path.exists(file_name):
            raise IOError(errno.ENOENT, os.strerror(errno.ENOENT), file_name)

        io.FileIO.__init__(self, file_name, mode)

        self.registers = Port.Registers(self)

        try:
            self.append_status = append_status
        except IOError as e:
            raise InvalidPortError(file_name)

    def purge(self, tx=True, rx=True):
        """Removes unsent and/or unread data from the card."""
        if (tx):
            try:
                fcntl.ioctl(self, FSCC_PURGE_TX)
            except IOError as e:
                raise e
        if (rx):
            try:
                fcntl.ioctl(self, FSCC_PURGE_RX)
            except IOError as e:
                raise e

    def _set_append_status(self, append_status):
        """Sets the value of the append status setting."""
        if append_status:
            fcntl.ioctl(self, FSCC_ENABLE_APPEND_STATUS)
        else:
            fcntl.ioctl(self, FSCC_DISABLE_APPEND_STATUS)

    def _get_append_status(self):
        """Gets the value of the append status setting."""
        buf = fcntl.ioctl(self, FSCC_GET_APPEND_STATUS,
                          struct.pack("I", 0))
        value = struct.unpack("I", buf)

        if (value[0]):
            return True
        else:
            return False

    append_status = property(fset=_set_append_status, fget=_get_append_status)

    def _set_memcap(self, input_memcap, output_memcap):
        """Sets the value of the memory cap setting."""
        fcntl.ioctl(self, FSCC_SET_MEMORY_CAP,
                    struct.pack("i" * 2, input_memcap, output_memcap))

    def _get_memcap(self):
        """Gets the value of the memory cap setting."""
        buf = fcntl.ioctl(self, FSCC_GET_MEMORY_CAP,
                          struct.pack("i" * 2, -1, -1))

        return struct.unpack("i" * 2, buf)

    def _set_imemcap(self, memcap):
        """Sets the value of the input memory cap setting."""
        self._set_memcap(memcap, -1)

    def _get_imemcap(self):
        """Gets the value of the output memory cap setting."""
        return self._get_memcap()[0]

    input_memory_cap = property(fset=_set_imemcap, fget=_get_imemcap)

    def _set_omemcap(self, memcap):
        """Sets the value of the output memory cap setting."""
        self._set_memcap(-1, memcap)

    def _get_omemcap(self):
        """Gets the value of the output memory cap setting."""
        return self._get_memcap()[1]

    output_memory_cap = property(fset=_set_omemcap, fget=_get_omemcap)

    def _set_ignore_timeout(self, ignore_timeout):
        """Sets the value of the ignore timeout setting."""
        if ignore_timeout:
            fcntl.ioctl(self, FSCC_ENABLE_IGNORE_TIMEOUT)
        else:
            fcntl.ioctl(self, FSCC_DISABLE_IGNORE_TIMEOUT)

    def _get_ignore_timeout(self):
        """Gets the value of the ignore timeout setting."""
        buf = fcntl.ioctl(self, FSCC_GET_IGNORE_TIMEOUT,
                          struct.pack("I", 0))
        value = struct.unpack("I", buf)

        return value[0]

    ignore_timeout = property(fset=_set_ignore_timeout,
                              fget=_get_ignore_timeout)

    def _set_tx_modifiers(self, tx_modifiers):
        """Sets the value of the transmit modifiers setting."""
        fcntl.ioctl(self, FSCC_SET_TX_MODIFIERS, tx_modifiers)

    def _get_tx_modifiers(self):
        """Gets the value of the transmit modifiers setting."""
        buf = fcntl.ioctl(self, FSCC_GET_TX_MODIFIERS,
                          struct.pack("I", 0))
        value = struct.unpack("I", buf)

        return value[0]

    tx_modifiers = property(fset=_set_tx_modifiers, fget=_get_tx_modifiers)

    def read(self, max_bytes=4096):
        """Reads data from the card."""
        if max_bytes:
            return super(io.FileIO, self).read(max_bytes)

    def can_read(self, timeout=100):
        """Checks whether there is data available to read."""
        poll_obj = select.poll()
        poll_obj.register(self, select.POLLIN)

        poll_data = poll_obj.poll(timeout)

        poll_obj.unregister(self)

        if poll_data and (poll_data[0][1] | select.POLLIN):
            return True
        else:
            return False

    def can_write(self, timeout=100):
        """Checks whether there is room available to write additional data."""
        poll_obj = select.poll()
        poll_obj.register(self, select.POLLOUT)

        poll_data = poll_obj.poll(timeout)

        poll_obj.unregister(self)

        if poll_data and (poll_data[0][1] | select.POLLOUT):
            return True
        else:
            return False
