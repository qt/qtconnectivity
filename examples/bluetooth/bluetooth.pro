TEMPLATE = subdirs
qtHaveModule(widgets) {
    SUBDIRS += btchat \
               btscanner \
               btfiletransfer \
               bttennis
}

SUBDIRS += scanner
