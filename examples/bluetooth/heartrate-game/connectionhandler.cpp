// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "connectionhandler.h"
#include "heartrate-global.h"

#include <QtBluetooth/qtbluetooth-config.h>

#include <QtCore/qsystemdetection.h>

ConnectionHandler::ConnectionHandler(QObject *parent) : QObject(parent)
{
    connect(&m_localDevice, &QBluetoothLocalDevice::hostModeStateChanged,
            this, &ConnectionHandler::hostModeChanged);
}

bool ConnectionHandler::alive() const
{

#ifdef QT_PLATFORM_UIKIT
    return true;

#else
    if (simulator)
        return true;
    return m_localDevice.isValid()
            && m_localDevice.hostMode() != QBluetoothLocalDevice::HostPoweredOff;
#endif
}

bool ConnectionHandler::requiresAddressType() const
{
#if QT_CONFIG(bluez)
    return true;
#else
    return false;
#endif
}

QString ConnectionHandler::name() const
{
    return m_localDevice.name();
}

QString ConnectionHandler::address() const
{
    return m_localDevice.address().toString();
}

void ConnectionHandler::hostModeChanged(QBluetoothLocalDevice::HostMode /*mode*/)
{
    emit deviceChanged();
}
