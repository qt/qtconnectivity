TEMPLATE = subdirs
SUBDIRS += auto

linux*:!linux-armcc:contains(bluez_enabled, yes):contains(QT_CONFIG, dbus) {
    SUBDIRS += btclient
}

!isEmpty(QT.nfc.name):SUBDIRS += nfctestserver
