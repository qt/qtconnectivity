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

#include "qbluetoothserviceinfo.h"
#include "qbluetoothserviceinfo_p.h"
#ifndef QT_IOS_BLUETOOTH
#include "dummy/dummy_helper_p.h"
#endif

QT_BEGIN_NAMESPACE

QBluetoothServiceInfoPrivate::QBluetoothServiceInfoPrivate()
{
#ifndef QT_IOS_BLUETOOTH
    printDummyWarning();
#endif
}

QBluetoothServiceInfoPrivate::~QBluetoothServiceInfoPrivate()
{
}

bool QBluetoothServiceInfoPrivate::isRegistered() const
{
    return false;
}

bool QBluetoothServiceInfoPrivate::registerService(const QBluetoothAddress &localAdapter)
{
    Q_UNUSED(localAdapter);
    return false;
}

bool QBluetoothServiceInfoPrivate::unregisterService()
{
    return false;
}

QT_END_NAMESPACE
