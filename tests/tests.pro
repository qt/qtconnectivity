TEMPLATE = subdirs
SUBDIRS += auto

linux*:!linux-armcc:contains(bluez_enabled, yes):qtHaveModule(dbus) {
    SUBDIRS += btclient
}

qtHaveModule(nfc): SUBDIRS += nfctestserver
