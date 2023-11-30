# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_find_package(PCSCLITE PROVIDED_TARGETS PkgConfig::PCSCLITE)

qt_feature("pcsclite" PUBLIC
    LABEL "Use the PCSCLite library to access NFC devices"
    CONDITION PCSCLITE_FOUND)

qt_feature("neard" PUBLIC
    LABEL "Use neard to access NFC devices"
    CONDITION LINUX AND NOT QT_FEATURE_pcsclite)
