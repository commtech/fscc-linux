"""
        Copyright (C) 2011 Commtech, Inc.

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

from generic import RegisterSequence, RegisterGUILegacy, RegisterGUI


class FIFOT(RegisterGUILegacy):

    def __init__(self, entry):
        super(FIFOT, self).__init__(entry)

        self.add_spin_button(range(16, 28), "Transmit FIFO Trigger Level (TXTRG)", 2 ** 12 - 1)
        self.add_spin_button(range(0, 13), "Transmit FIFO Trigger Level (TXTRG)", 2 ** 13 - 1)


class CCR0(RegisterGUI):
    bit_fields = [
        ("mode", range(0, 2)),
        ("cm", range(2, 5)),
        ("le", range(5, 8)),
        ("fsc", range(8, 11)),
        ("sflag", range(11, 12)),
        ("itf", range(12, 13)),
        ("nsb", range(13, 16)),
        ("ntb", range(16, 19)),
        ("vis", range(19, 20)),
        ("crc", range(20, 22)),
        ("obt", range(22, 23)),
        ("adm", range(23, 25)),
        ("recd", range(25, 26)),
        ("exts", range(28, 30)),
    ]
    
    glade_file = "ccr0.glade"
    
class CCR1(RegisterGUI):
    bit_fields = [
        ("rts", range(0, 1)),
        ("dtr", range(1, 2)),
        ("rtsc", range(2, 3)),
        ("ctsc", range(3, 4)),
        ("zins", range(4, 5)),
        ("oins", range(5, 6)),
        ("dps", range(6, 7)),
        ("sync2f", range(7, 8)),
        ("term2f", range(8, 9)),
        ("add2f", range(9, 10)),
        ("crc2f", range(10, 11)),
        ("crcr", range(11, 12)),
        ("drcrc", range(12, 13)),
        ("dtcrc", range(13, 14)),
        ("dterm", range(14, 15)),
        ("rip", range(16, 17)),
        ("cdp", range(17, 18)),
        ("rtsp", range(18, 19)),
        ("ctsp", range(19, 20)),
        ("dtrp", range(20, 21)),
        ("dsrp", range(21, 22)),
        ("fsrp", range(22, 23)),
        ("fstp", range(23, 24)),
        ("tcop", range(24, 25)),
        ("tcip", range(25, 26)),
        ("rcp", range(26, 27)),
        ("tdp", range(27, 28)),
        ("rdp", range(28, 29)),
    ]
    
    glade_file = "ccr1.glade"


class BGR(RegisterGUILegacy):

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


class PPR(RegisterGUILegacy):

    def __init__(self, entry):
        super(PPR, self).__init__(entry)

        self.add_spin_button(range(24, 32), "Number of Preambles (NPRE)", 2 ** 8 - 1)
        self.add_entry(range(16, 24), "Preamble (PRE)")
        self.add_spin_button(range(8, 16), "Number of Postambles (NPOST)", 2 ** 8 - 1)
        self.add_entry(range(0, 8), "Postamble (POST)")


class TCR(RegisterGUILegacy):
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


class FCR(RegisterGUILegacy):

    def __init__(self, entry):
        super(FCR, self).__init__(entry)

        self.add_check_button(range(16, 17), "Receive Echo Cancel Port A (RECHOA)")
        self.add_check_button(range(20, 21), "Receive Echo Cancel Port B (RECHOB)")
