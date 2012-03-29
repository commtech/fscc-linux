"""
    Copyright (C) 2011 Commtech, Inc.

    This file is part of fscc-gui.

    fscc-gui is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    fscc-gui is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with fscc-gui.  If not, see <http://www.gnu.org/licenses/>.

"""

import gtk
import pango

from generic import RegisterGUI


class FIFOT(RegisterGUI):
    bit_fields = [
        ("rxtrg", range(0, 13)),
        ("txtrg", range(16, 28)),
    ]
    
    glade_file = "fifot.glade"
    

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
    
    
"""
class CCR2(RegisterGUI):
    bit_fields = [
        ("fsro", range(0, 4)),
        ("fsto", range(4, 8)),
        ("rlc", range(16, 32)),
    ]
    
    glade_file = "ccr2.glade"
"""    
    
class BGR(RegisterGUI):
    bit_fields = [
        ("bgr", range(0, 32)),
    ]
    
    glade_file = "bgr.glade"
    

class SSR(RegisterGUI):
    bit_fields = [
        ("sync1", range(0, 8)),
        ("sync2", range(8, 16)),
        ("sync3", range(16, 24)),
        ("sync4", range(24, 32)),
    ]
    
    glade_file = "ssr.glade"


class SMR(RegisterGUI):
    bit_fields = [
        ("sm1", range(0, 8)),
        ("sm2", range(8, 16)),
        ("sm3", range(16, 24)),
        ("sm4", range(24, 32)),
    ]
    
    glade_file = "smr.glade"
    
    
class TSR(RegisterGUI):
    bit_fields = [
        ("term1", range(0, 8)),
        ("term2", range(8, 16)),
        ("term3", range(16, 24)),
        ("term4", range(24, 32)),
    ]
    
    glade_file = "tsr.glade"


class TMR(RegisterGUI):
    bit_fields = [
        ("tm1", range(0, 8)),
        ("tm2", range(8, 16)),
        ("tm3", range(16, 24)),
        ("tm4", range(24, 32)),
    ]
    
    glade_file = "tmr.glade"


class RAR(RegisterGUI):
    bit_fields = [
        ("gal", range(0, 8)),
        ("gah", range(8, 16)),
        ("sal", range(16, 24)),
        ("sah", range(24, 32)),
    ]
    
    glade_file = "rar.glade"


class RAMR(RegisterGUI):
    bit_fields = [
        ("galm", range(0, 8)),
        ("gahm", range(8, 16)),
        ("salm", range(16, 24)),
        ("sahm", range(24, 32)),
    ]
    
    glade_file = "ramr.glade"


class PPR(RegisterGUI):
    bit_fields = [
        ("post", range(0, 8)),
        ("npost", range(8, 16)),
        ("pre", range(16, 24)),
        ("npre", range(24, 32)),
    ]
    
    glade_file = "ppr.glade"


class TCR(RegisterGUI):
    bit_fields = [
        ("tsrc", range(0, 2)),
        ("ttrig", range(2, 3)),
        ("tcnt", range(3, 32)),
    ]
    
    glade_file = "tcr.glade"


class IMR(RegisterGUI):
    bit_fields = [
        ("rfs", range(0, 1)),
        ("rft", range(1, 2)),
        ("rfe", range(2, 3)),
        ("rfo", range(3, 4)),
        ("rdo", range(4, 5)),
        ("rfl", range(5, 6)),
        ("tin", range(8, 9)),
        ("dr_hi", range(10, 11)),
        ("dt_hi", range(11, 12)),
        ("dr_fe", range(12, 13)),
        ("dt_fe", range(13, 14)),
        ("dr_stop", range(14, 15)),
        ("dt_stop", range(15, 16)),
        ("tft", range(16, 17)),
        ("alls", range(17, 18)),
        ("tdu", range(18, 19)),
        ("ctss", range(24, 25)),
        ("dsrc", range(25, 26)),
        ("cdc", range(26, 27)),
        ("ctsa", range(27, 28)),
    ]
    
    glade_file = "imr.glade"


class DPLLR(RegisterGUI):
    bit_fields = [
        ("dpllr", range(0, 10)),
    ]
    
    glade_file = "dpllr.glade"


class FCR(RegisterGUI):
    bit_fields = [
        ("rechoa", range(16, 17)),
        ("tc485_a", range(17, 18)),
        ("td485_a", range(18, 19)),
        ("fstdtra", range(19, 20)),
        ("rechob", range(20, 21)),
        ("tc485_b", range(21, 22)),
        ("td485_b", range(22, 23)),
        ("fstdtrb", range(23, 24)),
        ("uarta", range(24, 25)),
        ("uartb", range(25, 26)),
    ]
    
    glade_file = "fcr.glade"
