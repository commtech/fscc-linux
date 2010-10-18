import gtk

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
