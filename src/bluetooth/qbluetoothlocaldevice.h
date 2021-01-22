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

#ifndef QBLUETOOTHLOCALDEVICE_H
#define QBLUETOOTHLOCALDEVICE_H

#include <QtBluetooth/qtbluetoothglobal.h>

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

#include <QtBluetooth/QBluetoothHostInfo>

QT_BEGIN_NAMESPACE

class QBluetoothLocalDevicePrivate;

class Q_BLUETOOTH_EXPORT QBluetoothLocalDevice : public QObject
{
    Q_OBJECT

public:
    enum Pairing {
        Unpaired,
        Paired,
        AuthorizedPaired
    };
    Q_ENUM(Pairing)

    enum HostMode {
        HostPoweredOff,
        HostConnectable,
        HostDiscoverable,
        HostDiscoverableLimitedInquiry
    };
    Q_ENUM(HostMode)

    enum Error {
        NoError,
        PairingError,
        UnknownError = 100
    };
    Q_ENUM(Error)

    explicit QBluetoothLocalDevice(QObject *parent = nullptr);
    explicit QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent = nullptr);
    virtual ~QBluetoothLocalDevice();

    bool isValid() const;

    void requestPairing(const QBluetoothAddress &address, Pairing pairing);
    Pairing pairingStatus(const QBluetoothAddress &address) const;

    void setHostMode(QBluetoothLocalDevice::HostMode mode);
    HostMode hostMode() const;
    QList<QBluetoothAddress> connectedDevices() const;

    void powerOn();

    QString name() const;
    QBluetoothAddress address() const;

    static QList<QBluetoothHostInfo> allDevices();

public Q_SLOTS:
    void pairingConfirmation(bool confirmation);

Q_SIGNALS:
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode state);
    void deviceConnected(const QBluetoothAddress &address);
    void deviceDisconnected(const QBluetoothAddress &address);
    void pairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);

    void pairingDisplayPinCode(const QBluetoothAddress &address, QString pin);
    void pairingDisplayConfirmation(const QBluetoothAddress &address, QString pin);
    void error(QBluetoothLocalDevice::Error error);

private:
    Q_DECLARE_PRIVATE(QBluetoothLocalDevice)
    QBluetoothLocalDevicePrivate *d_ptr;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QBluetoothLocalDevice::Pairing)
Q_DECLARE_METATYPE(QBluetoothLocalDevice::HostMode)
Q_DECLARE_METATYPE(QBluetoothLocalDevice::Error)

#endif // QBLUETOOTHLOCALDEVICE_H
