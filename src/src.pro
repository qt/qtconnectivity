TEMPLATE = subdirs

SUBDIRS += bluetooth nfc
android: SUBDIRS += android

!android {
    bluetooth_doc_snippets.subdir = bluetooth/doc/snippets
    bluetooth_doc_snippets.depends = bluetooth
    SUBDIRS += bluetooth_doc_snippets

    nfc_doc_snippets.subdir = nfc/doc/snippets
    nfc_doc_snippets.depends = nfc
    SUBDIRS += nfc_doc_snippets
}

qtHaveModule(quick) {
    imports.depends += bluetooth nfc
    SUBDIRS += imports
}
