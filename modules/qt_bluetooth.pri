QT.bluetooth.VERSION = 5.0.0
QT.bluetooth.MAJOR_VERSION = 5
QT.bluetooth.MINOR_VERSION = 0
QT.bluetooth.PATCH_VERSION = 0

QT.bluetooth.name = QtBluetooth
QT.bluetooth.bins = $$QT_MODULE_BIN_BASE
QT.bluetooth.includes = $$QT_MODULE_INCLUDE_BASE $$QT_MODULE_INCLUDE_BASE/QtBluetooth
QT.bluetooth.private_includes = $$QT_MODULE_INCLUDE_BASE/QtBluetooth/$$QT.bluetooth.VERSION
QT.bluetooth.sources = $$QT_MODULE_BASE/src/bluetooth
QT.bluetooth.libs = $$QT_MODULE_LIB_BASE
QT.bluetooth.plugins = $$QT_MODULE_PLUGIN_BASE
QT.bluetooth.imports = $$QT_MODULE_IMPORT_BASE
QT.bluetooth.depends = core
QT.bluetooth.DEFINES = QT_BLUETOOTH_LIB

QT_CONFIG += bluetooth
