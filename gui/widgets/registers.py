"""
        Copyright (C) 2010  Commtech, Inc.

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

import gtk
import pango

from generic import RegisterSequence, RegisterGUI


class FIFOT(RegisterGUI):

    def __init__(self, entry):
        super(FIFOT, self).__init__(entry)

        self.add_spin_button(range(16, 28), "Transmit FIFO Trigger Level (TXTRG)", 2 ** 12 - 1)
        self.add_spin_button(range(0, 13), "Transmit FIFO Trigger Level (TXTRG)", 2 ** 13 - 1)


class CCR0(RegisterGUI):
    modes = [
        "HDLC",
        "X-Sync",
        "Transparent",
    ]

    les = [
        "NRZ",
        "NRZI",
        "FM0",
        "FM1",
        "Manchester",
        "Differential Manchester",
    ]

    fscs = [
        "None",
        "Mode 1",
        "Mode 2",
        "Mode 3",
        "Mode 2 & 3",
    ]

    itfs = [
        "Logical 1's",
        "SYNC Sequences",
    ]

    crcs = [
        "CRC-8",
        "CRC-CCITT",
        "CRC-16",
        "CRC-32",
    ]

    def __init__(self, entry):
        super(CCR0, self).__init__(entry)

        self.add_combo_box(range(0, 2), "Operating Mode (MODE)", CCR0.modes)
        self.add_spin_button(range(2, 5), "Clock Mode (CM)", 7)
        self.add_combo_box(range(5, 8), "Line Encoding (LE)", CCR0.les)
        self.add_combo_box(range(8, 11), "Frame Sync Control (FSC)", CCR0.fscs)
        self.add_check_button(range(11, 12), "Shared Flags (SFLAG)")
        self.add_radio_buttons(range(12, 13), "Inter-frame Time Fill (ITF)", CCR0.itfs)
        self.add_spin_button(range(13, 16), "Number of Sync Bytes (NSB)", 4)
        self.add_spin_button(range(16, 19), "Number of Termination Bytes (NTB)", 4)
        self.add_check_button(range(19, 20), "Masked Interrupts Visible (VIS)")
        self.add_combo_box(range(20, 22), "CRC Frame Check Mode (CRC)", CCR0.crcs)


class BGR(RegisterGUI):

    def __init__(self, entry):
        super(BGR, self).__init__(entry)

        self.add_spin_button(range(32), "Baud Rate Divisor (BGR)", 2 ** 32 - 1)


class SSR(RegisterSequence):

    def __init__(self, entry):
        labels = ["Sync Byte %i" % i for i in range(1, 5)]

        super(SSR, self).__init__(entry, labels)


class TSR(RegisterSequence):

    def __init__(self, entry):
        labels = ["Term Byte %i" % i for i in range(1, 5)]

        super(TSR, self).__init__(entry, labels)


class RAR(RegisterSequence):

    def __init__(self, entry):
        labels = [
            "Group Addr-L",
            "Group Addr-H",
            "Station Addr-L",
            "Station Addr-H",
        ]

        super(RAR, self).__init__(entry, labels)


class RAMR(RegisterSequence):

    def __init__(self, entry):
        labels = [
            "Group Addr-L Mask",
            "Group Addr-H Mask",
            "Station Addr-L Mask",
            "Station Addr-H Mask",
        ]

        super(RAMR, self).__init__(entry, labels)


class PPR(RegisterGUI):

    def __init__(self, entry):
        super(PPR, self).__init__(entry)

        self.add_spin_button(range(24, 32), "Number of Preambles (NPRE)", 2 ** 8 - 1)
        self.add_entry(range(16, 24), "Preamble (PRE)")
        self.add_spin_button(range(8, 16), "Number of Postambles (NPOST)", 2 ** 8 - 1)
        self.add_entry(range(0, 8), "Postamble (POST)")


class TCR(RegisterGUI):
    tsrcs = [
        "OSC Input",
        "Transmit Clock",
        "Receive Clock",
        "PCI Bus Clock",
    ]

    ttrigs = [
        "Continuous",
        "Once",
    ]

    def __init__(self, entry):
        super(TCR, self).__init__(entry)

        self.add_combo_box(range(0, 2), "Timer Clock Source (TSRC)", TCR.tsrcs)
        self.add_radio_buttons(range(2, 3), "Timer Trigger (TTRIG)", TCR.ttrigs)
        self.add_spin_button(range(3, 32), "Timer Expiration Count (TCNT)", 2 ** 29 - 1)


class FCR(RegisterGUI):

    def __init__(self, entry):
        super(FCR, self).__init__(entry)

        self.add_check_button(range(16, 17), "Receive Echo Cancel Port A (RECHOA)")
        self.add_check_button(range(20, 21), "Receive Echo Cancel Port B (RECHOB)")
