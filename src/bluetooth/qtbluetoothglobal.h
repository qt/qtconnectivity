/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/
#ifndef QTBLUETOOTH_H
#define QTBLUETOOTH_H

#include <QtCore/qglobal.h>
#include <QtBluetooth/qtbluetooth-config.h> // for feature detection

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#    if defined(QT_BUILD_BLUETOOTH_LIB)
#      define Q_BLUETOOTH_EXPORT Q_DECL_EXPORT
#    else
#      define Q_BLUETOOTH_EXPORT Q_DECL_IMPORT
#    endif
#else
#    define Q_BLUETOOTH_EXPORT
#endif

QT_END_NAMESPACE

#endif // QTBLUETOOTH_H
