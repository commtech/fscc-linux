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

    def get_value(self):
        return self.entry.get_value()

    def set_value(self, value):
        self.entry.set_value(value)
        self.set_sensitive(True)

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

    def get_value(self):
        return int(self.get_text(), 16)

    def set_value(self, value):
        self.set_text("%08x" % value)
        self.emit("activate")


class RegisterGUI(gtk.VBox):

    def __init__(self, entry):
        super(RegisterGUI, self).__init__(False, 10)

        self.spin_buttons = []
        self.entries = []
        self.combo_boxes = []
        self.check_buttons = []
        self.radio_buttons = []

        self.entry = entry
        self.entry.connect("activate", self.entry_changed_base)
        self.entry.connect("focus-out-event", self.entry_changed_base)

    def clear_bits(self, old_value, bits):
        mask = old_value

        for bit in bits:
            mask &= ~(1 << bit);

        return mask

    def entry_changed_base(self, widget, data=None):
        if self.entry and self.entry.get_text():
            entry_value = int(self.entry.get_text(), 16)

            for spin_button, bits in self.spin_buttons:
                opposite_bits = [i for i in range(32) if i not in bits]
                mask = self.clear_bits(entry_value, opposite_bits)
                spin_button.set_value(mask >> bits[0])

            for entry, bits in self.entries:
                opposite_bits = [i for i in range(32) if i not in bits]
                mask = self.clear_bits(entry_value, opposite_bits)
                entry.set_text("%02x" % (mask >> bits[0]))

            for combo_box, bits in self.combo_boxes:
                opposite_bits = [i for i in range(32) if i not in bits]
                mask = self.clear_bits(entry_value, opposite_bits)
                combo_box.set_active(mask >> bits[0])

            for check_button, bits in self.check_buttons:
                opposite_bits = [i for i in range(32) if i not in bits]
                mask = self.clear_bits(entry_value, opposite_bits)
                check_button.set_active(mask >> bits[0])

            for j, (radio_button, bits) in enumerate(self.radio_buttons, start=2):
                opposite_bits = [i for i in range(32) if i not in bits]
                mask = self.clear_bits(entry_value, opposite_bits)

                if j % 2 == 0:
                    radio_button.set_active(not (mask >> bits[0]))
                else:
                    radio_button.set_active(mask >> bits[0])

        self.entry_changed(widget, data)

    def _add_widget(self, widget, label):
        frame = gtk.Frame(label)
        frame.add(widget)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        self.pack_start(frame)

    def _update_entry_value(self, widget_value, bits):
        old_entry_value = int(self.entry.get_text(), 16)
        mask = self.clear_bits(old_entry_value, bits)
        new_entry_value = (mask | ( widget_value << bits[0]))

        self.entry.set_text("%08x" % new_entry_value)

    def entry_changed(self, widget, data=None):
        pass

    def add_radio_buttons(self, bits, label, options):
        hbox = gtk.HBox(False, 25)
        hbox.show()

        for i, option in enumerate(options):
            if i == 0:
                radio_button = gtk.RadioButton(label=option)
                radio_button.connect("toggled", self._radio_button_toggled, bits)
            else:
                radio_button = gtk.RadioButton(label=option, group=self.radio_buttons[-1][0])

            radio_button.show()
            hbox.pack_start(radio_button)
            self.radio_buttons.append((radio_button, bits))

        self._add_widget(hbox, label)

    def add_check_button(self, bits, label):
        check_button = gtk.CheckButton()
        check_button.show()

        check_button.connect("toggled", self._check_button_toggled, bits)

        self.check_buttons.append((check_button, bits))
        self._add_widget(check_button, label)

    def add_combo_box(self, bits, label, options):
        combo_box = gtk.combo_box_new_text()

        for option in options:
            combo_box.append_text(option)

        combo_box.show()

        combo_box.connect("changed", self._combo_box_changed, bits)

        self.combo_boxes.append((combo_box, bits))
        self._add_widget(combo_box, label)

    def add_entry(self, bits, label):
        entry = gtk.Entry()
        entry.show()

        entry.connect("activate", self._sub_entry_activate, bits)
        entry.connect("focus-out-event", self._sub_entry_focus_out_event, bits)

        self.entries.append((entry, bits))
        self._add_widget(entry, label)

    def add_spin_button(self, bits, label, max_value):
        adjustment = gtk.Adjustment(0, 0, max_value, 1, 0, 0)

        spin_button = gtk.SpinButton(adjustment, 1, 0)
        spin_button.show()

        spin_button.connect("value-changed", self._spin_button_value_changed, bits)

        self.spin_buttons.append((spin_button, bits))
        self._add_widget(spin_button, label)

    def _radio_button_toggled(self, widget, bits):
        self._update_entry_value(not widget.get_active(), bits)

    def _check_button_toggled(self, widget, bits):
        self._update_entry_value(widget.get_active(), bits)

    def _combo_box_changed(self, widget, bits):
        self._update_entry_value(widget.get_active(), bits)

    def _sub_entry_activate(self, widget, bits):
        self._update_entry_value(int(widget.get_text(), 16), bits)

    def _sub_entry_focus_out_event(self, widget, event, bits):
        self._update_entry_value(int(widget.get_text(), 16), bits)

    def _spin_button_value_changed(self, widget, bits):
        self._update_entry_value(widget.get_value_as_int(), bits)


class RegisterSequence(RegisterGUI):

    def __init__(self, entry, labels):
        super(RegisterSequence, self).__init__(entry)

        for i, label in enumerate(labels):
            self.add_entry(range(i * 8, i * 8 + 8), label)
