/****************************************************************************
**
** Copyright (C) 2017 Governikus GmbH & Co. KG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

#include <QCoreApplication>

#include "qnearfieldtarget_p.h"
#include "qnearfieldtarget_android_p.h"

QT_BEGIN_NAMESPACE

bool QNearFieldTargetPrivate::keepConnection() const
{
    NEARFIELDTARGET_Q();
    return q->keepConnection();
}

bool QNearFieldTargetPrivate::setKeepConnection(bool isPersistent)
{
    NEARFIELDTARGET_Q();
    return q->setKeepConnection(isPersistent);
}

bool QNearFieldTargetPrivate::disconnect()
{
    NEARFIELDTARGET_Q();
    return q->disconnect();
}

int QNearFieldTargetPrivate::maxCommandLength() const
{
    NEARFIELDTARGET_Q();
    return q->maxCommandLength();
}

QT_END_NAMESPACE
