TEMPLATE = subdirs

SUBDIRS += heartrate-server

qtHaveModule(widgets) {
    SUBDIRS += btchat \
               btscanner
}

qtHaveModule(quick): SUBDIRS += scanner \
                                pingpong \
                                lowenergyscanner \
                                heartrate-game \
                                chat
