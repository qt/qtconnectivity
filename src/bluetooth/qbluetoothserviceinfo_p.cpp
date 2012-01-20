/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothserviceinfo.h"
#include "qbluetoothserviceinfo_p.h"

QTBLUETOOTH_BEGIN_NAMESPACE

bool QBluetoothServiceInfo::isRegistered() const
{
    return false;
}

bool QBluetoothServiceInfo::registerService() const
{
    return false;
}

bool QBluetoothServiceInfo::unregisterService() const
{
    return false;
}

QBluetoothServiceInfoPrivate::QBluetoothServiceInfoPrivate()
{
}

QBluetoothServiceInfoPrivate::~QBluetoothServiceInfoPrivate()
{
}

void QBluetoothServiceInfoPrivate::setRegisteredAttribute(quint16 attributeId, const QVariant &value) const
{
    Q_UNUSED(attributeId);
    Q_UNUSED(value);
}

void QBluetoothServiceInfoPrivate::removeRegisteredAttribute(quint16 attributeId) const
{
    Q_UNUSED(attributeId);
}

#ifdef QT_BLUEZ_BLUETOOTH
bool QBluetoothServiceInfoPrivate::registerService() const
{
    return false;
}
#endif

QTBLUETOOTH_END_NAMESPACE
