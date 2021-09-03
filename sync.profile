%modules = ( # path to module name map
    "QtBluetooth" => "$basedir/src/bluetooth",
    "QtNfc" => "$basedir/src/nfc",
);
%moduleheaders = ( # restrict the module headers to those found in relative path
);

@ignore_for_include_check = (

    # BlueZ auto-generated headers
    "adapter1_bluez5_p.h", "device1_bluez5_p.h", "objectmanager_p.h",
    "profile1_p.h", "properties_p.h", "gattchar1_p.h",
    "gattdesc1_p.h", "gattservice1_p.h", "profilemanager1_p.h", "battery1_p.h"
);
