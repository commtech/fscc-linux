import gtk
import pango

import fscc


class Register(gtk.HBox):

    def __init__(self, register_name, port=None):
        super(Register, self).__init__(False, 0)

        label = gtk.Label(register_name)
        label.set_width_chars(6)
        label.set_alignment(1, 0.5)
        label.show()

        self.entry = RegisterEntry(register_name, port)
        self.entry.show()

        image = gtk.Image()
        image.set_from_stock(gtk.STOCK_EDIT, gtk.ICON_SIZE_BUTTON)

        self.pack_start(label, False, False, 10)
        self.pack_start(self.entry, False, False, 0)

        self.set_sensitive(False)

    def update_value(self, port=None):
        self.entry.update_value(port)
        self.set_sensitive(True)

    def save_value(self):
        self.entry.save_value()

    def get_value(self):
        return self.entry.get_value()

    def set_verbose_widget(self, widget):
        self.verbose_widget = widget

    def hide_gui(self):
        if getattr(self, "verbose_widget", None):
            self.verbose_widget.hide()

    def show_gui(self):
        if getattr(self, "verbose_widget", None):
            self.verbose_widget.show()


class RegisterEntry(gtk.Entry):

    def __init__(self, register_name, port=None):
        super(RegisterEntry, self).__init__()

        self.register_name = register_name
        self.port = port

        self.set_max_length(8)
        self.set_width_chars(8)

        #font_desc = pango.FontDescription("monospace")
        #self.modify_font(font_desc)

        if self.port:
            self.update_value()

    def update_value(self, port=None):
        if port:
            self.port = port

        self.port.reset_registers()
        setattr(self.port, self.register_name, fscc.FSCC_UPDATE_VALUE)
        self.port.get_registers()

        self.set_text("%08x" % getattr(self.port, self.register_name))

        self.emit("activate")

    def save_value(self):
        self.port.reset_registers()
        setattr(self.port, self.register_name, int(self.get_text(), 16))
        self.port.set_registers()

    def get_value(self):
        return self.get_text()


class RegisterSequence(gtk.VBox):

    def __init__(self, entry, labels):
        super(RegisterSequence, self).__init__(False, 10)

        self.labels = labels

        for i, label in enumerate(self.labels):
            byte_entry = gtk.Entry()
            setattr(self, "byte_%i" % i, byte_entry)
            byte_entry.set_width_chars(2)
            byte_entry.show()

            #font_desc = pango.FontDescription('monospace')
            #byte_entry.modify_font(font_desc)

            byte_entry.connect("activate", self.byte_entry_changed)
            byte_entry.connect("focus-out-event", self.byte_entry_changed)

            frame = gtk.Frame(label)
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

            for i, label in enumerate(self.labels):
                byte_entry = getattr(self, "byte_%i" % i)
                byte_entry.set_text("%02x" % ((entry_value >> (i * 8)) & 0x000000ff))

    def byte_entry_changed(self, widget, data=None):
        new_value = 0

        for i, label in enumerate(self.labels):
            byte_entry_value = int(getattr(self, "byte_%i" % i).get_text(), 16)
            new_value |= (byte_entry_value << (i * 8))

        self.entry.set_text("%08x" % new_value)
