# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#####################################################################
## tst_qlowenergycontroller-gattserver Test:
#####################################################################

qt_internal_add_test(tst_qlowenergycontroller-gattserver
    SOURCES
        tst_qlowenergycontroller-gattserver.cpp
    LIBRARIES
        Qt::Bluetooth
        Qt::BluetoothPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qlowenergycontroller-gattserver CONDITION QT_FEATURE_linux_crypto_api
    DEFINES
        CONFIG_LINUX_CRYPTO_API
)

qt_internal_extend_target(tst_qlowenergycontroller-gattserver CONDITION QT_FEATURE_bluez_le
    DEFINES
        CONFIG_BLUEZ_LE
)

qt_internal_extend_target(tst_qlowenergycontroller-gattserver CONDITION boot2qt
    DEFINES
        CHECK_CMAC_SUPPORT
)
