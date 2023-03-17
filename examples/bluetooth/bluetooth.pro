TEMPLATE = subdirs

SUBDIRS += heartrate-server

qtHaveModule(widgets) {
    SUBDIRS += btchat
}

qtHaveModule(quick): SUBDIRS += pingpong \
                                lowenergyscanner \
                                heartrate-game
