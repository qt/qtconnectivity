include(FindPkgConfig)

pkg_check_modules(BLUEZ bluez IMPORTED_TARGET)

if (NOT TARGET PkgConfig::BLUEZ)
    set(BLUEZ_FOUND 0)
endif()
