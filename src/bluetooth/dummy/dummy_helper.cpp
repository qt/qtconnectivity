/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include <QtCore/QGlobalStatic>
#include <QtCore/QLoggingCategory>

#include "dummy_helper_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT)

Q_GLOBAL_STATIC_WITH_ARGS(bool, dummyWarningPrinted, (false))

void printDummyWarning()
{
    if (!*dummyWarningPrinted()) {
        qCWarning(QT_BT) << "Dummy backend running. Qt Bluetooth module is non-functional.";
        *dummyWarningPrinted() = true;
    }
}

QT_END_NAMESPACE
