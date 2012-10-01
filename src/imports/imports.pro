TEMPLATE = subdirs

!isEmpty(QT.bluetooth.name):SUBDIRS += bluetooth
!isEmpty(QT.nfc.name):SUBDIRS += nfc
