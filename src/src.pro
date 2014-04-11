TEMPLATE = subdirs

SUBDIRS += bluetooth nfc
android: SUBDIRS += android

contains(QT_CONFIG, private_tests) {
    bluetooth_doc_snippets.subdir = bluetooth/doc/snippets
    bluetooth_doc_snippets.depends = bluetooth

    nfc_doc_snippets.subdir = nfc/doc/snippets
    nfc_doc_snippets.depends = nfc

    !contains(QT_BUILD_PARTS, examples) {
        bluetooth_doc_snippets.CONFIG = no_default_target no_default_install
        nfc_doc_snippets.CONFIG = no_default_target no_default_install
    }

    SUBDIRS += bluetooth_doc_snippets nfc_doc_snippets
}

qtHaveModule(quick) {
    imports.depends += bluetooth nfc
    SUBDIRS += imports
}
