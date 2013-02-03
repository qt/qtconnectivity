/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QBLUETOOTHDEVICEDISCOVERYAGENT_H
#define QBLUETOOTHDEVICEDISCOVERYAGENT_H

#include "qbluetoothglobal.h"

#include <QObject>

#include <qbluetoothdeviceinfo.h>

QT_BEGIN_NAMESPACE_BLUETOOTH

class QBluetoothDeviceDiscoveryAgentPrivate;

class Q_BLUETOOTH_EXPORT QBluetoothDeviceDiscoveryAgent : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QBluetoothDeviceDiscoveryAgent::InquiryType inquiryType READ inquiryType WRITE setInquiryType)

public:
    // FIXME: add more errors
    // FIXME: add bluez error handling
    enum Error {
        NoError,
        IOFailure,
        PoweredOff,
        UnknownError = 100
    };

    enum InquiryType {
        GeneralUnlimitedInquiry,
        LimitedInquiry,
    };

    QBluetoothDeviceDiscoveryAgent(QObject *parent = 0);
    ~QBluetoothDeviceDiscoveryAgent();

    QBluetoothDeviceDiscoveryAgent::InquiryType inquiryType() const;
    void setInquiryType(QBluetoothDeviceDiscoveryAgent::InquiryType type);

    bool isActive() const;

    Error error() const;
    QString errorString() const;

    QList<QBluetoothDeviceInfo> discoveredDevices() const;

public slots:
    void start();
    void stop();

signals:
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
    void finished();
    void error(QBluetoothDeviceDiscoveryAgent::Error error);
    void canceled();

private:
    Q_DECLARE_PRIVATE(QBluetoothDeviceDiscoveryAgent)
    QBluetoothDeviceDiscoveryAgentPrivate *d_ptr;

#ifdef QT_BLUEZ_BLUETOOTH
    Q_PRIVATE_SLOT(d_func(), void _q_deviceFound(const QString &address, const QVariantMap &dict));
    Q_PRIVATE_SLOT(d_func(), void _q_propertyChanged(const QString &name, const QDBusVariant &value));
#endif
};

QT_END_NAMESPACE_BLUETOOTH

#endif
