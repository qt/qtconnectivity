/***************************************************************************
**
** Copyright (C) 2012 Research In Motion
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothserviceinfo.h"
#include "qbluetoothserviceinfo_p.h"

#include "qbluetoothserver_p.h"
#include "qbluetoothserver.h"

QT_BEGIN_NAMESPACE

QBluetoothServiceInfoPrivate::QBluetoothServiceInfoPrivate()
:  registered(false)
{
}

QBluetoothServiceInfoPrivate::~QBluetoothServiceInfoPrivate()
{
}

bool QBluetoothServiceInfoPrivate::isRegistered() const
{
    return registered;
}

bool QBluetoothServiceInfoPrivate::unregisterService()
{
    if (!registered)
        return false;

    return false;
}

extern QHash<QBluetoothServerPrivate*, int> __fakeServerPorts;

bool QBluetoothServiceInfoPrivate::registerService(const QBluetoothAddress& localAdapter)
{
    Q_UNUSED(localAdapter); //QNX always uses default local adapter
    if (protocolDescriptor(QBluetoothUuid::Rfcomm).isEmpty()) {
        qCWarning(QT_BT_QNX) << Q_FUNC_INFO << "Only SPP services can be registered on QNX";
        return false;
    }

    if (serverChannel() == -1)
        return false;

    if (__fakeServerPorts.key(serverChannel()) != 0) {
        if (!ppsSendControlMessage("register_server", 0x1101, attributes.value(QBluetoothServiceInfo::ServiceId).value<QBluetoothUuid>(), QString(),
                                   attributes.value(QBluetoothServiceInfo::ServiceName).toString(),
                              __fakeServerPorts.key(serverChannel()), BT_SPP_SERVER_SUBTYPE))
            return false;
        //The server needs to know the service name for the socket mount point path
        __fakeServerPorts.key(serverChannel())->m_serviceName = attributes.value(QBluetoothServiceInfo::ServiceName).toString();
    } else {
        return false;
    }

    registered = true;
    return true;
}

QT_END_NAMESPACE
