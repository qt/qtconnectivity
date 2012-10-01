TEMPLATE = subdirs
CONFIG  += ordered
SUBDIRS = bluetooth imports

# The Qt NFC module is currently not supported.  The API is not finalized and may change.
# The Qt NFC module can be built by passing CONFIG+=nfc to qmake.
nfc {
    message("Building unsupported Qt NFC module, API is not finalized and may change.")
    SUBDIRS += nfc
}
