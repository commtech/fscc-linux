import gtk

import config


class AboutDialog(gtk.AboutDialog):

    def __init__(self):
        super(AboutDialog, self).__init__()

        self.set_name(config.NAME)
        self.set_program_name(config.PROGRAM_NAME)
        self.set_version(config.VERSION)
        self.set_website("www.commtech-fastcom.com")
        self.set_website_label("Commtech Website")
        self.set_authors(["William Fagan <willf@commtech-fastcom.com>"])


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
