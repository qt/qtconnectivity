TEMPLATE = subdirs

SUBDIRS += heartrate-server

qtHaveModule(widgets) {
    SUBDIRS += btchat \
               btscanner
}

qtHaveModule(quick): SUBDIRS += pingpong \
                                lowenergyscanner \
                                heartrate-game
