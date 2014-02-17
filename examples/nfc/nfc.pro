TEMPLATE = subdirs
qtHaveModule(widgets) {
    SUBDIRS += \
        annotatedurl \
        ndefeditor \
        poster
}

qtHaveModule(quick): SUBDIRS += corkboard
