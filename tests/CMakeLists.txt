# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

if(QT_BUILD_STANDALONE_TESTS)
    # Add qt_find_package calls for extra dependencies that need to be found when building
    # the standalone tests here.
endif()

if(TARGET Qt::Bluetooth)
    if(TARGET Qt::Quick)
        add_subdirectory(bttestui)
    endif()

    add_subdirectory(bluetoothtestdevice)
endif()

qt_build_tests()
