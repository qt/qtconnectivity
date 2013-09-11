TEMPLATE = subdirs
CONFIG  += ordered

SUBDIRS += bluetooth nfc
qtHaveModule(quick): SUBDIRS += imports
