TEMPLATE = subdirs
!contains(QT_CONFIG, no-widgets) {
    SUBDIRS += btchat \
               btscanner \
               btfiletransfer \
               bttennis
}

SUBDIRS += scanner
