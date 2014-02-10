TEMPLATE = subdirs
CONFIG  += ordered

SUBDIRS += bluetooth nfc
android: SUBDIRS += android
qtHaveModule(quick): SUBDIRS += imports
