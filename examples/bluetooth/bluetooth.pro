TEMPLATE = subdirs
qtHaveModule(widgets) {
    SUBDIRS += btchat \
               btscanner \
               btfiletransfer \
               bttennis
}

qtHaveModule(quick): SUBDIRS += scanner
