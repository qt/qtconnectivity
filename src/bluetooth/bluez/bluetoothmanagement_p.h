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

#ifndef BLUETOOTHMANAGEMENT_P_H
#define BLUETOOTHMANAGEMENT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qdatetime.h>
#include <QtCore/qmutex.h>
#include <QtCore/qobject.h>

#include <QtBluetooth/qbluetoothaddress.h>

#ifndef QPRIVATELINEARBUFFER_BUFFERSIZE
#define QPRIVATELINEARBUFFER_BUFFERSIZE Q_INT64_C(16384)
#endif
#include "../qprivatelinearbuffer_p.h"

QT_BEGIN_NAMESPACE

class QSocketNotifier;

class BluetoothManagement : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothManagement(QObject *parent = nullptr);
    static BluetoothManagement *instance();

    bool isAddressRandom(const QBluetoothAddress &address) const;
    bool isMonitoringEnabled() const;

private slots:
    void _q_readNotifier();
    void processRandomAddressFlagInformation(const QBluetoothAddress &address);
    void cleanupOldAddressFlags();

private:
    void readyRead();

    int fd = -1;
    QSocketNotifier* notifier;
    QPrivateLinearBuffer buffer;
    QHash<QBluetoothAddress, QDateTime> privateFlagAddresses;
    mutable QMutex accessLock;
};


QT_END_NAMESPACE

#endif // BLUETOOTHMANAGEMENT_P_H
