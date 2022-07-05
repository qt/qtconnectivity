# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_find_package(PCSCLite PROVIDED_TARGETS PkgConfig::PCSCLite)

qt_feature("pcsclite" PUBLIC
    LABEL "PCSCLite"
    CONDITION PCSCLITE_FOUND)
