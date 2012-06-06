TEMPLATE = subdirs
!contains(QT_CONFIG, no-widgets) {
    SUBDIRS += btchat \
               btscanner \
               btfiletransfer \
               bttennis
}

SUBDIRS += scanner

#Qt NFC based examples
#SUBDIRS += poster \
#           ndefeditor \
#           annotatedurl
