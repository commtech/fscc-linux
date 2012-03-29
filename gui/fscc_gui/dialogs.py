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
import gobject
import threading

import info


class AboutDialog(gtk.AboutDialog):

    def __init__(self):
        super(AboutDialog, self).__init__()

        self.set_program_name(info.__program_name__)
        self.set_version(info.__version__)
        self.set_website("http://code.google.com/p/fscc-gui/")
        self.set_website_label("Website")
        self.set_authors(["William Fagan <willf@commtech-fastcom.com>"])
        self.set_copyright("Copyright (C) 2011 Commtech, Inc.")
        self.set_license("""
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

    """)


class OpenPortDialog(gtk.FileChooserDialog):

    def __init__(self):
        buttons = (
            gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
            gtk.STOCK_OPEN, gtk.RESPONSE_OK,
        )

        arguments = (
            "Open Port",
            None,
            gtk.FILE_CHOOSER_ACTION_OPEN,
            buttons,
        )

        super(OpenPortDialog, self).__init__(*arguments)

        self.set_current_folder("/dev/")
        self.set_filename("/dev/fscc0")

    def get_port_name(self):
        if self.run() == gtk.RESPONSE_OK:
            port_name = self.get_filename()
        else:
            port_name = None

        self.destroy()

        return port_name


class ExportSettingsDialog(gtk.FileChooserDialog):

    def __init__(self):
        buttons = (
            gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
            gtk.STOCK_SAVE, gtk.RESPONSE_OK,
        )

        arguments = (
            "Export Settings",
            None,
            gtk.FILE_CHOOSER_ACTION_SAVE,
            buttons,
        )

        super(ExportSettingsDialog, self).__init__(*arguments)


class ImportSettingsDialog(gtk.FileChooserDialog):

    def __init__(self):
        buttons = (
            gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
            gtk.STOCK_OPEN, gtk.RESPONSE_OK,
        )

        arguments = (
            "Import Settings",
            None,
            gtk.FILE_CHOOSER_ACTION_OPEN,
            buttons,
        )

        super(ImportSettingsDialog, self).__init__(*arguments)

class InvalidPortDialog(gtk.MessageDialog):

    def __init__(self):
        arguments = (
            None,
            gtk.DIALOG_MODAL & gtk.DIALOG_DESTROY_WITH_PARENT,
            gtk.MESSAGE_ERROR,
            gtk.BUTTONS_CLOSE,
            "That doesn't appear to be a valid FSCC port.",
        )

        super(InvalidPortDialog, self).__init__(*arguments)

class InsufficientPermissionsDialog(gtk.MessageDialog):

    def __init__(self):
        arguments = (
            None,
            gtk.DIALOG_MODAL & gtk.DIALOG_DESTROY_WITH_PARENT,
            gtk.MESSAGE_ERROR,
            gtk.BUTTONS_CLOSE,
            "You don't have sufficient permissions to open that port.",
        )

        super(InsufficientPermissionsDialog, self).__init__(*arguments)

class SettingsExportedDialog(gtk.MessageDialog):

    def __init__(self):
        arguments = (
            None,
            gtk.DIALOG_MODAL & gtk.DIALOG_DESTROY_WITH_PARENT,
            gtk.MESSAGE_INFO,
            gtk.BUTTONS_CLOSE,
            "Your settings have been exported.",
        )

        super(SettingsExportedDialog, self).__init__(*arguments)


class SettingsImportedDialog(gtk.MessageDialog):

    def __init__(self):
        arguments = (
            None,
            gtk.DIALOG_MODAL & gtk.DIALOG_DESTROY_WITH_PARENT,
            gtk.MESSAGE_INFO,
            gtk.BUTTONS_CLOSE,
            "Your settings have been imported.",
        )

        super(SettingsImportedDialog, self).__init__(*arguments)


class SettingsSavedDialog(gtk.MessageDialog):

    def __init__(self):
        arguments = (
            None,
            gtk.DIALOG_MODAL & gtk.DIALOG_DESTROY_WITH_PARENT,
            gtk.MESSAGE_INFO,
            gtk.BUTTONS_CLOSE,
            "Your settings have been saved.",
        )

        super(SettingsSavedDialog, self).__init__(*arguments)


class InvalidValuesDialog(gtk.MessageDialog):

    def __init__(self, values):
        if len(values) == 1:
            message = "The following register contains an invalid " \
                      "hexadecimal value. It must be fixed before " \
                      "continuing.\n"
        else:
            message = "The following registers contain an invalid " \
                      "hexadecimal value. They must be fixed before " \
                      "continuing.\n"

        for register_name, value in values:
            message += "\n%s - 0x%s" % (register_name, value)

        arguments = (
            None,
            gtk.DIALOG_MODAL & gtk.DIALOG_DESTROY_WITH_PARENT,
            gtk.MESSAGE_ERROR,
            gtk.BUTTONS_CLOSE,
            message,
        )

        super(InvalidValuesDialog, self).__init__(*arguments)
        

class TerminalDialog(gtk.Dialog):
    class ReadThread(threading.Thread, gobject.GObject):
        def __init__(self, port):
            threading.Thread.__init__(self)
            gobject.GObject.__init__(self)

            self.port = port

        def run(self):
            while True:
                text = self.port.read(4096)
                self.emit("changed", text)

    def __init__(self, port):
        buttons = (
            gtk.STOCK_CLOSE,
            gtk.RESPONSE_CLOSE,
        )

        super(TerminalDialog, self).__init__("FSCC Send/Receive", buttons=buttons)

        self.set_border_width(10)
        self.set_default_size(600, 400)

        paned = gtk.VPaned()
        paned.set_position(200)
        paned.show()

        self.outgoing_text = gtk.TextView()
        self.outgoing_text.show()

        send_button = gtk.Button("Send")
        send_button.connect("clicked", self.send_clicked)
        send_button.show()

        button_box = gtk.HButtonBox()
        button_box.set_layout(gtk.BUTTONBOX_END)
        button_box.set_spacing(5)
        button_box.show()
        button_box.pack_start(send_button, False, False)

        vbox = gtk.VBox()
        vbox.show()
        vbox.pack_start(self.outgoing_text, True, True, 5)
        vbox.pack_start(button_box, False, False, 5)

        paned.add(vbox)

        self.incoming_text = gtk.Label()
        self.incoming_text.set_alignment(0, 0)
        self.incoming_text.show()

        vbox = gtk.VBox()
        vbox.set_border_width(5)
        vbox.pack_start(self.incoming_text, True, True)
        vbox.show()

        scrolled_window = gtk.ScrolledWindow()
        scrolled_window.add_with_viewport(vbox)
        scrolled_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        scrolled_window.set_shadow_type(gtk.SHADOW_NONE)
        scrolled_window.show()

        frame = gtk.Frame("Incoming")
        frame.add(scrolled_window)
        frame.set_shadow_type(gtk.SHADOW_NONE)
        frame.show()

        paned.add(frame)

        self.vbox.pack_start(paned, True, True, 5)

        self.port = port

        read_thread = TerminalDialog.ReadThread(self.port)
        read_thread.connect("changed", self.incoming_text_changed)
        read_thread.daemon = True
        read_thread.start()

    def send_clicked(self, widget, data=None):
        text_buffer = self.outgoing_text.get_buffer()
        start, end = text_buffer.get_bounds()

        text = text_buffer.get_text(start, end)
        text_buffer.delete(start, end)

        self.port.write(text)

    def incoming_text_changed(self, widget, text):
        self.incoming_text.set_text(text)


gobject.type_register(TerminalDialog.ReadThread)

gobject.signal_new("changed", TerminalDialog.ReadThread, gobject.SIGNAL_RUN_FIRST,
                   gobject.TYPE_NONE, (gobject.TYPE_STRING,))
