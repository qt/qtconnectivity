# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_example(heartrate-server)
if(TARGET Qt::Widgets)
    qt_internal_add_example(btchat)
endif()
if(TARGET Qt::Quick)
    qt_internal_add_example(lowenergyscanner)
    qt_internal_add_example(heartrate-game)
endif()
