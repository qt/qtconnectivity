TEMPLATE = subdirs
CONFIG  += ordered

SUBDIRS += bluetooth nfc
qtHaveModules(quick): SUBDIRS += imports
