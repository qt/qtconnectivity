requires(!android|qtHaveModule(androidextras))

load(configure)
qtCompileTest(bluez)
qtCompileTest(bluez_le)
qtCompileTest(linux_crypto_api)
load(qt_parts)
