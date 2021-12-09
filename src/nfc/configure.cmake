qt_find_package(PCSCLite PROVIDED_TARGETS PkgConfig::PCSCLite)

qt_feature("pcsclite" PUBLIC
    LABEL "PCSCLite"
    CONDITION PCSCLITE_FOUND)
