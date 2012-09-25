TEMPLATE = subdirs
!contains(QT_CONFIG, no-widgets) {
    SUBDIRS += bluetooth
}

SUBDIRS += bluetooth

#Qt NFC based examples
#SUBDIRS += poster \
#           ndefeditor \
#           annotatedurl
