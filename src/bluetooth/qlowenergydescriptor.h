/***************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Copyright (C) 2016 BlackBerry Limited. All rights reserved.
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

#ifndef QLOWENERGYDESCRIPTOR_H
#define QLOWENERGYDESCRIPTOR_H

#include <QtCore/QSharedPointer>
#include <QtCore/QVariantMap>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/QBluetoothUuid>

QT_BEGIN_NAMESPACE

struct QLowEnergyDescriptorPrivate;
class QLowEnergyServicePrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyDescriptor
{
public:
    QLowEnergyDescriptor();
    QLowEnergyDescriptor(const QLowEnergyDescriptor &other);
    ~QLowEnergyDescriptor();

    QLowEnergyDescriptor &operator=(const QLowEnergyDescriptor &other);
    bool operator==(const QLowEnergyDescriptor &other) const;
    bool operator!=(const QLowEnergyDescriptor &other) const;

    bool isValid() const;

    QByteArray value() const;

    QBluetoothUuid uuid() const;
    QLowEnergyHandle handle() const;
    QString name() const;

    QBluetoothUuid::DescriptorType type() const;

protected:
    QLowEnergyHandle characteristicHandle() const;
    QSharedPointer<QLowEnergyServicePrivate> d_ptr;

    friend class QLowEnergyCharacteristic;
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
    QLowEnergyDescriptorPrivate *data = nullptr;

    QLowEnergyDescriptor(QSharedPointer<QLowEnergyServicePrivate> p,
                             QLowEnergyHandle charHandle,
                             QLowEnergyHandle descHandle);
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QLowEnergyDescriptor)

#endif // QLOWENERGYDESCRIPTOR_H
