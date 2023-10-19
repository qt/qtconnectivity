# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial

qt_find_package(PCSCLITE PROVIDED_TARGETS PkgConfig::PCSCLITE)

qt_feature("pcsclite" PUBLIC
    LABEL "PCSCLite"
    CONDITION PCSCLITE_FOUND)
