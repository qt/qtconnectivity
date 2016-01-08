requires(!android|qtHaveModule(androidextras))

load(configure)
qtCompileTest(bluez)
qtCompileTest(bluez_le)
load(qt_parts)
