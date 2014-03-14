TEMPLATE = subdirs
SUBDIRS += auto

qtHaveModule(bluetooth):qtHaveModule(quick): SUBDIRS += bttestui

qtHaveModule(nfc): SUBDIRS += nfctestserver
