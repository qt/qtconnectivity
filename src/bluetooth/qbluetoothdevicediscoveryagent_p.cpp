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

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"

#include "dummy/dummy_helper_p.h"

#define QT_DEVICEDISCOVERY_DEBUG

QT_BEGIN_NAMESPACE

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
                const QBluetoothAddress &deviceAdapter,
                QBluetoothDeviceDiscoveryAgent *parent)
    :   inquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry),
        lastError(QBluetoothDeviceDiscoveryAgent::NoError),
        lowEnergySearchTimeout(-1),
        q_ptr(parent)
{
    Q_UNUSED(deviceAdapter);
    printDummyWarning();
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    return false;
}

QBluetoothDeviceDiscoveryAgent::DiscoveryMethods QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods()
{
    return QBluetoothDeviceDiscoveryAgent::NoMethod;
}

void QBluetoothDeviceDiscoveryAgentPrivate::start(QBluetoothDeviceDiscoveryAgent::DiscoveryMethods)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    lastError = QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError;
    errorString = QBluetoothDeviceDiscoveryAgent::tr("Device discovery not supported on this platform");

    emit q->error(lastError);
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
}



QT_END_NAMESPACE

#include "moc_qbluetoothdevicediscoveryagent_p.cpp"
