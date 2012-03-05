%modules = ( # path to module name map
    "QtBluetooth" => "$basedir/src/bluetooth",
#    "QtNfc" => "$basedir/src/nfc",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);
%classnames = (
    "qtbluetoothversion.h" => "QtBluetoothVersion",
#    "qtnfcversion.h" => "QtNfcVersion",
);
%mastercontent = (
    "bluetooth" => "#include <QtBluetooth/QtBluetooth>\n",
#    "nfc" => "#include <QtNfc/QtNfc>\n",
);
%modulepris = (
    "QtBluetooth" => "$basedir/modules/qt_bluetooth.pri",
#    "QtNfc" => "$basedir/modules/qt_nfc.pri",
);
# Module dependencies.
# Every module that is required to build this module should have one entry.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - any git symbolic ref resolvable from the module's repository (e.g. "refs/heads/master" to track master branch)
#
%dependencies = (
    "qtbase" => "refs/heads/master",
    "qtdeclarative" => "refs/heads/master",
    "qtjsbackend" => "refs/heads/master",
    "qtscript" => "refs/heads/master",
    "qtsvg" => "refs/heads/master",
    "qtsystems" => "refs/heads/master",
    "qtxmlpatterns" => "refs/heads/master",
);

# Compile tests
%configtests = (
    "bluez" => {},
);
