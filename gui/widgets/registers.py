import gtk
import pango

from generic import RegisterSequence


class FIFOT(gtk.VBox):

    def __init__(self, entry):
        super(FIFOT, self).__init__(False, 10)

        register_sections = [
            ("Transmit FIFO Trigger Level", "TXTRG", 4095),
            ("Receive FIFO Trigger Level", "RXTRG", 8191),
        ]

        for name, abbr, max_value in register_sections:
            adjustment = gtk.Adjustment(0, 0, max_value, 1, 0, 0)

            spin_button = gtk.SpinButton(adjustment, 1, 0)
            spin_button.connect("value-changed",
                                getattr(self, "%s_value_changed" % abbr.lower()))
            spin_button.show()

            frame = gtk.Frame("%s (%s)" % (name, abbr))
            frame.add(spin_button)
            frame.set_shadow_type(gtk.SHADOW_NONE)
            frame.show()

            self.pack_start(frame)

            setattr(self, abbr.lower(), spin_button)

        self.entry = entry
        self.entry.connect("activate", self.entry_changed)
        self.entry.connect("focus-out-event", self.entry_changed)

    def entry_changed(self, widget, data=None):
        if self.entry and self.entry.get_text():
            getattr(self, "txtrg").set_value(int(self.entry.get_text(), 16) >> 16)
            getattr(self, "rxtrg").set_value(int(self.entry.get_text(), 16) & 0x00001fff)

    def txtrg_value_changed(self, widget, data=None):
        txtrg_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0x00001fff) | (txtrg_value << 16)))

    def rxtrg_value_changed(self, widget, data=None):
        rxtrg_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0x0fff0000) | rxtrg_value))


class CCR0(gtk.VBox):
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

    def __init__(self, entry):
        super(CCR0, self).__init__(False, 10)

        self.mode = gtk.combo_box_new_text()

        for mode in CCR0.modes:
            self.mode.append_text(mode)

        self.mode.connect("changed", self.mode_changed)
        self.mode.show()

        frame = gtk.Frame("Operating Mode (MODE)")
        frame.add(self.mode)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        adjustment = gtk.Adjustment(0, 0, 7, 1, 0, 0)

        self.cm = gtk.SpinButton(adjustment, 1, 0)
        self.cm.connect("value-changed", self.cm_value_changed)
        self.cm.show()

        frame = gtk.Frame("Clock Mode (CM)")
        frame.add(self.cm)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        self.le = gtk.combo_box_new_text()

        for le in CCR0.les:
            self.le.append_text(le)

        self.le.connect("changed", self.le_changed)
        self.le.show()

        frame = gtk.Frame("Line Encoding (LE)")
        frame.add(self.le)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        self.fsc = gtk.combo_box_new_text()

        for fsc in CCR0.fscs:
            self.fsc.append_text(fsc)

        self.fsc.connect("changed", self.fsc_changed)
        self.fsc.show()

        frame = gtk.Frame("Frame Sync Control (FSC)")
        frame.add(self.fsc)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        self.sflag = gtk.CheckButton()
        self.sflag.connect("toggled", self.sflag_toggled)
        self.sflag.show()

        frame = gtk.Frame("Shared Flags (SFLAG)")
        frame.add(self.sflag)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        self.itf_ones = gtk.RadioButton(label="Logical 1's")
        self.itf_ones.connect("toggled", self.itf_ones_toggled)
        self.itf_ones.show()

        self.itf_flags = gtk.RadioButton(self.itf_ones, label="SYNC Sequences")
        self.itf_flags.show()

        hbox = gtk.HBox(False, 25)
        hbox.pack_start(self.itf_ones)
        hbox.pack_start(self.itf_flags)
        hbox.show()

        frame = gtk.Frame("Inter-frame Time Fill (ITF)")
        frame.add(hbox)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        adjustment = gtk.Adjustment(0, 0, 4, 1, 0, 0)

        self.nsb = gtk.SpinButton(adjustment, 1, 0)
        self.nsb.connect("value-changed", self.nsb_value_changed)
        self.nsb.show()

        frame = gtk.Frame("Number of Sync Bytes (NSB)")
        frame.add(self.nsb)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        adjustment = gtk.Adjustment(0, 0, 4, 1, 0, 0)

        self.ntb = gtk.SpinButton(adjustment, 1, 0)
        self.ntb.connect("value-changed", self.ntb_value_changed)
        self.ntb.show()

        frame = gtk.Frame("Number of Termination Bytes (NTB)")
        frame.add(self.ntb)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        self.entry = entry
        self.entry.connect("activate", self.entry_changed)
        self.entry.connect("focus-out-event", self.entry_changed)

    def entry_changed(self, widget, data=None):
        if self.entry and self.entry.get_text():
            self.mode.set_active(int(self.entry.get_text(), 16) & 0x00000003)
            self.cm.set_value((int(self.entry.get_text(), 16) & 0x0000001c) >> 2)
            self.le.set_active((int(self.entry.get_text(), 16) & 0x000000e0) >> 5)
            self.fsc.set_active((int(self.entry.get_text(), 16) & 0x00000700) >> 8)
            self.sflag.set_active((int(self.entry.get_text(), 16) & 0x00000800) >> 11)
            self.itf_ones.set_active(not ((int(self.entry.get_text(), 16) & 0x00001000) >> 12))
            self.itf_flags.set_active((int(self.entry.get_text(), 16) & 0x00001000) >> 12)
            self.nsb.set_value((int(self.entry.get_text(), 16) & 0x0000e000) >> 13)
            self.ntb.set_value((int(self.entry.get_text(), 16) & 0x00070000) >> 16)

    def mode_changed(self, widget, data=None):
        mode_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xfffffffc) | mode_value))

    def cm_value_changed(self, widget, data=None):
        cm_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xffffffe3) | (cm_value << 2)))

    def le_changed(self, widget, data=None):
        le_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xffffff1f) | (le_value << 5)))

    def fsc_changed(self, widget, data=None):
        fsc_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xfffff8ff) | (fsc_value << 8)))

    def sflag_toggled(self, widget, data=None):
        sflag_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xfffff7ff) | (sflag_value << 11)))

    def itf_ones_toggled(self, widget, data=None):
        itf_ones_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xffffefff) | ((not itf_ones_value) << 12)))

    def nsb_value_changed(self, widget, data=None):
        nsb_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xffff1fff) | (nsb_value << 13)))

    def ntb_value_changed(self, widget, data=None):
        ntb_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xfff8ffff) | (ntb_value << 16)))


class BGR(gtk.VBox):

    def __init__(self, entry):
        super(BGR, self).__init__(False, 10)

        adjustment = gtk.Adjustment(0, 0, 2 ** 32 - 1, 1, 0, 0)

        self.bgr = gtk.SpinButton(adjustment, 1, 0)
        self.bgr.connect("value-changed", self.bgr_value_changed)
        self.bgr.show()

        frame = gtk.Frame("Baud Rate Divisor (BGR)")
        frame.add(self.bgr)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        self.entry = entry
        self.entry.connect("activate", self.entry_changed)
        self.entry.connect("focus-out-event", self.entry_changed)

    def entry_changed(self, widget, data=None):
        if self.entry and self.entry.get_text():
            self.bgr.set_value(int(self.entry.get_text(), 16))

    def bgr_value_changed(self, widget, data=None):
        bgr_value = widget.get_value()
        self.entry.set_text("%08x" % bgr_value)


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


class PPR(gtk.VBox):

    def __init__(self, entry):
        super(PPR, self).__init__(False, 10)

        adjustment = gtk.Adjustment(0, 0, 2 ** 8 - 1, 1, 0, 0)

        self.npre = gtk.SpinButton(adjustment, 1, 0)
        self.npre.connect("value-changed", self.npre_value_changed)
        self.npre.show()

        frame = gtk.Frame("Number of Preambles (NPRE)")
        frame.add(self.npre)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        self.pre = gtk.Entry()
        self.pre.show()

        #font_desc = pango.FontDescription('monospace')
        #self.pre.modify_font(font_desc)

        self.pre.connect("activate", self.pre_changed)
        self.pre.connect("focus-out-event", self.pre_changed)

        frame = gtk.Frame("Preamble (PRE)")
        frame.add(self.pre)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        adjustment = gtk.Adjustment(0, 0, 2 ** 8 - 1, 1, 0, 0)

        self.npost = gtk.SpinButton(adjustment, 1, 0)
        self.npost.connect("value-changed", self.npost_value_changed)
        self.npost.show()

        frame = gtk.Frame("Number of Postambles (NPOST)")
        frame.add(self.npost)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        self.post = gtk.Entry()
        self.post.show()

        #font_desc = pango.FontDescription('monospace')
        #self.post.modify_font(font_desc)

        self.post.connect("activate", self.post_changed)
        self.post.connect("focus-out-event", self.post_changed)

        frame = gtk.Frame("Postamble (POST)")
        frame.add(self.post)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        self.entry = entry
        self.entry.connect("activate", self.entry_changed)
        self.entry.connect("focus-out-event", self.entry_changed)

    def entry_changed(self, widget, data=None):
        if self.entry and self.entry.get_text():
            entry_value = int(self.entry.get_text(), 16)
            self.npre.set_value((entry_value & 0xff000000) >> 24)
            self.pre.set_text("%02x" % ((entry_value & 0x00ff0000) >> 16))
            self.npost.set_value((entry_value & 0x0000ff00) >> 8)
            self.post.set_text("%02x" % (entry_value & 0x000000ff))

    def npre_value_changed(self, widget, data=None):
        npre_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0x00ffffff) | (npre_value << 24)))

    def npost_value_changed(self, widget, data=None):
        npost_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xffff00ff) | (npost_value << 8)))

    def pre_changed(self, widget, data=None):
        pre_value = int(widget.get_text(), 16)
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xff00ffff) | (pre_value << 16)))

    def post_changed(self, widget, data=None):
        post_value = int(widget.get_text(), 16)
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xffffff00) | post_value))


class TCR(gtk.VBox):
    tsrcs = [
        "OSC Input",
        "Transmit Clock",
        "Receive Clock",
        "PCI Bus Clock",
    ]

    def __init__(self, entry):
        super(TCR, self).__init__(False, 10)

        self.tsrc = gtk.combo_box_new_text()

        for tsrc in TCR.tsrcs:
            self.tsrc.append_text(tsrc)

        self.tsrc.connect("changed", self.tsrc_changed)
        self.tsrc.show()

        frame = gtk.Frame("Timer Clock Source (TSRC)")
        frame.add(self.tsrc)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        self.ttrig_recycle = gtk.RadioButton(label="Continuous")
        self.ttrig_recycle.connect("toggled", self.ttrig_recycle_toggled)
        self.ttrig_recycle.show()

        self.ttrig_once = gtk.RadioButton(self.ttrig_recycle, label="Once")
        self.ttrig_once.show()

        hbox = gtk.HBox(False, 25)
        hbox.pack_start(self.ttrig_recycle)
        hbox.pack_start(self.ttrig_once)
        hbox.show()

        frame = gtk.Frame("Timer Trigger (TTRIG)")
        frame.add(hbox)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        adjustment = gtk.Adjustment(0, 0, 2 ** 29 - 1, 1, 0, 0)

        self.tcnt = gtk.SpinButton(adjustment, 1, 0)
        self.tcnt.connect("value-changed", self.tcnt_value_changed)
        self.tcnt.show()

        frame = gtk.Frame("Timer Expiration Count (TCNT)")
        frame.add(self.tcnt)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

        self.entry = entry
        self.entry.connect("activate", self.entry_changed)
        self.entry.connect("focus-out-event", self.entry_changed)

    def entry_changed(self, widget, data=None):
        if self.entry and self.entry.get_text():
            self.tsrc.set_active(int(self.entry.get_text(), 16) & 0x00000003)
            self.ttrig_recycle.set_active(not ((int(self.entry.get_text(), 16) & 0x00000004) >> 2))
            self.ttrig_once.set_active((int(self.entry.get_text(), 16) & 0x00000004) >> 2)
            self.tcnt.set_value((int(self.entry.get_text(), 16) & 0xfffffff8) >> 3)

    def tsrc_changed(self, widget, data=None):
        tsrc_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xfffffffc) | tsrc_value))

    def ttrig_recycle_toggled(self, widget, data=None):
        ttrig_recycle_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xfffffffb) | ((not ttrig_recycle_value) << 2)))

    def tcnt_value_changed(self, widget, data=None):
        tcnt_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0x00000007) | (tcnt_value << 3)))
