TEMPLATE = subdirs
qtHaveModule(widgets) {
    SUBDIRS += \
        annotatedurl \
        ndefeditor
}
