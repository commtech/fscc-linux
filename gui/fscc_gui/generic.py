"""
    Copyright (C) 2012 Commtech, Inc.

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

import os

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
    bit_fields = []
    glade_file = ""
    
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
                
        builder = gtk.Builder()
        builder.add_from_file(os.path.dirname(__file__) + "/layouts/" + self.glade_file) #TODO: Fix this
        
        container = builder.get_object("register")
        self._add_widget(container)
        
        self._setup_widgets(builder.get_objects())

    def clear_bits(self, old_value, bits):
        mask = old_value

        for bit in bits:
            mask &= ~(1 << bit);

        return mask
               
    def _setup_widgets(self, widgets):
        bit_field_names = [name for name, bits in self.bit_fields]
        bit_field_bits = [bits for name, bits in self.bit_fields]
        
        for widget in widgets:
            try:
                name = gtk.Buildable.get_name(widget)
                
                if name in bit_field_names:
                    bits = self.bit_fields[bit_field_names.index(name)][1]
                else:
                    continue
            except:
                continue
                
            group = None
                
            if type(widget) is gtk.SpinButton:
                widget.connect("value-changed", self._spin_button_value_changed, bits)
                group = self.spin_buttons
                
            if type(widget) is gtk.HScale:
                widget.connect("value-changed", self._spin_button_value_changed, bits)
                group = self.spin_buttons
                
            if type(widget) is gtk.ComboBox:
                widget.connect("changed", self._combo_box_changed, bits)
                group = self.combo_boxes
                
            if type(widget) is gtk.CheckButton:
                widget.connect("toggled", self._check_button_toggled, bits)
                group = self.check_buttons
                
            if type(widget) is gtk.Entry:
                widget.connect("activate", self._sub_entry_activate, bits)
                widget.connect("focus-out-event", self._sub_entry_focus_out_event, bits)
                group = self.entries
                
            if type(widget) is gtk.RadioButton:
            	widget.connect("toggled", self._radio_button_toggled, bits)           	
                group = self.radio_buttons
                
            if group is not None:
                group.append((widget, bits))

    def _add_widget(self, widget):
        if widget:
            self.pack_start(widget, False, False)

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
            
            for radio_button, bits in self.radio_buttons:
                opposite_bits = [i for i in range(32) if i not in bits]
                mask = self.clear_bits(entry_value, opposite_bits)
                
                if radio_button.get_group()[0] is radio_button:
                    other_button = radio_button.get_group()[1]
                else:
                    other_button = radio_button.get_group()[0]
                    
                if mask >> bits[0]:
                    other_button.set_active(1)
                else:
                    radio_button.set_active(1)

        self.entry_changed(widget, data)

    def entry_changed(self, widget, data=None):
        pass

    def _combo_box_changed(self, widget, bits):
        self._update_entry_value(widget.get_active(), bits)

    def _spin_button_value_changed(self, widget, bits):
        self._update_entry_value(int(widget.get_value()), bits)

    def _check_button_toggled(self, widget, bits):
        self._update_entry_value(widget.get_active(), bits)

    def _sub_entry_activate(self, widget, bits):
        self._update_entry_value(int(widget.get_text(), 16), bits)

    def _sub_entry_focus_out_event(self, widget, event, bits):
        self._update_entry_value(int(widget.get_text(), 16), bits)

    def _radio_button_toggled(self, widget, bits):
        self._update_entry_value(not widget.get_active(), bits)

    def _update_entry_value(self, widget_value, bits):
        try:
            old_entry_value = int(self.entry.get_text(), 16)
        except:
            pass
        else:
            mask = self.clear_bits(old_entry_value, bits)
            new_entry_value = (mask | ( widget_value << bits[0]))

            self.entry.set_text("%08x" % new_entry_value)
