TEMPLATE = subdirs
!contains(QT_CONFIG, no-widgets) {
    SUBDIRS += \
        annotatedurl \
        ndefeditor \
        poster
}
