TEMPLATE = subdirs
SUBDIRS += auto

linux*:!linux-armcc:contains(bluez_enabled, yes):qtHaveModule(dbus) {
    SUBDIRS += btclient
}

qtHaveModule(bluetooth):qtHaveModule(quick): SUBDIRS += bttestui

qtHaveModule(nfc): SUBDIRS += nfctestserver
