import gtk


class FIFOT(gtk.VBox):
    def __init__(self, entry):
        super(FIFOT, self).__init__(False, 10)

        register_sections  = [
            ("TXTRG", 4095),
            ("RXTRG", 8191),
        ]

        for name, max_value in register_sections:
            adjustment = gtk.Adjustment(0, 0, max_value, 1, 0, 0)

            spin_button = gtk.SpinButton(adjustment, 1, 0)
            spin_button.connect("value-changed",
                                getattr(self, "%s_value_changed" % name.lower()))
            spin_button.show()

            frame = gtk.Frame(name)
            frame.add(spin_button)
            frame.set_shadow_type(gtk.SHADOW_NONE)
            frame.show()

            self.pack_start(frame)

            setattr(self, name.lower(), spin_button)

        self.entry = entry
        self.entry.connect("activate", self.entry_changed)
        self.entry.connect("focus-out-event", self.entry_changed)

    def entry_changed(self, widget, data=None):
        if self.entry and self.entry.get_text():
            getattr(self, "txtrg").set_value(int(self.entry.get_text(), 16) >> 16)
            getattr(self, "rxtrg").set_value(int(self.entry.get_text(), 16) & 0x00001fff)

    def txtrg_value_changed(self, widget, data=None):
        txtrg_value = widget.get_value_as_int() << 16
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0x00001fff) + txtrg_value))

    def rxtrg_value_changed(self, widget, data=None):
        rxtrg_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0x0fff0000) + rxtrg_value))


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

        frame = gtk.Frame("MODE")
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

        self.entry.set_text("%08x" % ((entry_value & 0xfffffffc) + mode_value))

    def cm_value_changed(self, widget, data=None):
        cm_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xffffffe3) + (cm_value << 2)))

    def le_changed(self, widget, data=None):
        le_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xffffff1f) + (le_value << 5)))

    def fsc_changed(self, widget, data=None):
        fsc_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xfffff8ff) + (fsc_value << 8)))

    def sflag_toggled(self, widget, data=None):
        sflag_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xfffff7ff) + (sflag_value << 11)))

    def itf_ones_toggled(self, widget, data=None):
        itf_ones_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xffffefff) + ((not itf_ones_value) << 12)))

    def nsb_value_changed(self, widget, data=None):
        nsb_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xffff1fff) + (nsb_value << 13)))

    def ntb_value_changed(self, widget, data=None):
        ntb_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xfff8ffff) + (ntb_value << 16)))

class BGR(gtk.VBox):
    def __init__(self, entry):
        super(BGR, self).__init__(False, 10)

        adjustment = gtk.Adjustment(0, 0, 2 ** 32, 1, 0, 0)

        self.bgr = gtk.SpinButton(adjustment, 1, 0)
        self.bgr.connect("value-changed", self.bgr_value_changed)
        self.bgr.show()

        frame = gtk.Frame("BGR")
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
        bgr_value = widget.get_value_as_int()
        self.entry.set_text("%08x" % bgr_value)

class SequenceRegister(gtk.VBox):
    def __init__(self, entry):
        super(SequenceRegister, self).__init__(False, 10)

        for i in range(4):
            byte_entry = gtk.Entry()
            setattr(self, "byte_%i" % i, byte_entry)
            byte_entry.show()

            byte_entry.connect("activate", self.byte_entry_changed)
            byte_entry.connect("focus-out-event", self.byte_entry_changed)

            frame = gtk.Frame("Byte %i" % (i + 1))
            frame.add(byte_entry)
            frame.set_shadow_type(gtk.SHADOW_NONE)
            frame.show()

            self.pack_start(frame)

        self.entry = entry
        self.entry.connect("activate", self.entry_changed)
        self.entry.connect("focus-out-event", self.entry_changed)

    def entry_changed(self, widget, data=None):
        if self.entry and self.entry.get_text():
            entry_value = int(self.entry.get_text(), 16)

            for i in range(4):
                byte_entry = getattr(self, "byte_%i" % i)
                byte_entry.set_text("%02x" % ((entry_value >> (i * 8)) & 0x000000ff))

    def byte_entry_changed(self, widget, data=None):
        new_value = 0

        for i in range(4):
            byte_entry_value = int(getattr(self, "byte_%i" % i).get_text(), 16)
            new_value += (byte_entry_value << (i * 8))

        self.entry.set_text("%08x" % new_value)

class SSR(SequenceRegister):
    def __init__(self, entry):
        super(SSR, self).__init__(entry)

class TSR(SequenceRegister):
    def __init__(self, entry):
        super(TSR, self).__init__(entry)
