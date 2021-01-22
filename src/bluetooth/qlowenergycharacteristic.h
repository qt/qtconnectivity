/***************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Copyright (C) 2016 BlackBerry Limited all rights reserved
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

#ifndef QLOWENERGYCHARACTERISTIC_H
#define QLOWENERGYCHARACTERISTIC_H
#include <QtCore/QSharedPointer>
#include <QtCore/QObject>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QLowEnergyDescriptor>

QT_BEGIN_NAMESPACE

class QBluetoothUuid;
class QLowEnergyServicePrivate;
struct QLowEnergyCharacteristicPrivate;
class Q_BLUETOOTH_EXPORT QLowEnergyCharacteristic
{
public:

    enum PropertyType {
        Unknown = 0x00,
        Broadcasting = 0x01,
        Read = 0x02,
        WriteNoResponse = 0x04,
        Write = 0x08,
        Notify = 0x10,
        Indicate = 0x20,
        WriteSigned = 0x40,
        ExtendedProperty = 0x80
    };
    Q_DECLARE_FLAGS(PropertyTypes, PropertyType)

    QLowEnergyCharacteristic();
    QLowEnergyCharacteristic(const QLowEnergyCharacteristic &other);
    ~QLowEnergyCharacteristic();

    QLowEnergyCharacteristic &operator=(const QLowEnergyCharacteristic &other);
    bool operator==(const QLowEnergyCharacteristic &other) const;
    bool operator!=(const QLowEnergyCharacteristic &other) const;

    QString name() const;

    QBluetoothUuid uuid() const;

    QByteArray value() const;

    QLowEnergyCharacteristic::PropertyTypes properties() const;
    QLowEnergyHandle handle() const;

    QLowEnergyDescriptor descriptor(const QBluetoothUuid &uuid) const;
    QList<QLowEnergyDescriptor> descriptors() const;

    bool isValid() const;

protected:
    QLowEnergyHandle attributeHandle() const;

    QSharedPointer<QLowEnergyServicePrivate> d_ptr;

    friend class QLowEnergyService;
    friend class QLowEnergyControllerPrivate;
    friend class QLowEnergyControllerPrivateAndroid;
    friend class QLowEnergyControllerPrivateBluez;
    friend class QLowEnergyControllerPrivateBluezDBus;
    friend class QLowEnergyControllerPrivateCommon;
    friend class QLowEnergyControllerPrivateWin32;
    friend class QLowEnergyControllerPrivateDarwin;
    friend class QLowEnergyControllerPrivateWinRT;
    friend class QLowEnergyControllerPrivateWinRTNew;
    QLowEnergyCharacteristicPrivate *data = nullptr;
    QLowEnergyCharacteristic(QSharedPointer<QLowEnergyServicePrivate> p,
                             QLowEnergyHandle handle);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QLowEnergyCharacteristic::PropertyTypes)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QLowEnergyCharacteristic)

#endif // QLOWENERGYCHARACTERISTIC_H
