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

#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothservicediscoveryagent_p.h"
#ifndef QT_IOS_BLUETOOTH
#include "dummy/dummy_helper_p.h"
#endif

QT_BEGIN_NAMESPACE

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(
    QBluetoothServiceDiscoveryAgent *qp, const QBluetoothAddress &deviceAdapter)
    : error(QBluetoothServiceDiscoveryAgent::NoError),
      state(Inactive),
      mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery),
      singleDevice(false),
      q_ptr(qp)
{
#ifndef QT_IOS_BLUETOOTH
    printDummyWarning();
#endif
    Q_UNUSED(deviceAdapter);
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    Q_UNUSED(address);
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
}

QT_END_NAMESPACE
