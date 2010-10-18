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

        frame = gtk.Frame("CM")
        frame.add(self.cm)
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

    def mode_changed(self, widget, data=None):
        mode_value = widget.get_active()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xfffffffc) + mode_value))

    def cm_value_changed(self, widget, data=None):
        cm_value = widget.get_value_as_int()
        entry_value = int(self.entry.get_text(), 16)

        self.entry.set_text("%08x" % ((entry_value & 0xffffffe3) + (cm_value << 2)))


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
