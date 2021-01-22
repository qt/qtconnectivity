/****************************************************************************
**
** Copyright (C) 2017 Governikus GmbH & Co. K
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

#include "qnearfieldtarget.h"
#include "qnearfieldtarget_p.h"

QT_BEGIN_NAMESPACE

bool QNearFieldTargetPrivate::keepConnection() const
{
    return false;
}

bool QNearFieldTargetPrivate::setKeepConnection(bool isPersistent)
{
    Q_UNUSED(isPersistent);
    return false;
}

bool QNearFieldTargetPrivate::disconnect()
{
    return false;
}

int QNearFieldTargetPrivate::maxCommandLength() const
{
    return 0;
}

QT_END_NAMESPACE
