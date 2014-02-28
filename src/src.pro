TEMPLATE = subdirs

SUBDIRS += bluetooth nfc
android: SUBDIRS += android

qtHaveModule(quick) {
    imports.depends += bluetooth nfc
    SUBDIRS += imports
}
