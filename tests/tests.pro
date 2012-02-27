TEMPLATE = subdirs
SUBDIRS += auto

# NFC disabled
#SUBDIRS += nfctestserver

linux*:!linux-armcc:contains(bluez_enabled, yes):contains(QT_CONFIG, dbus) {
    SUBDIRS += btclient
}
