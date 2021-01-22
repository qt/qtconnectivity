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

#include "qlowenergycontroller_dummy_p.h"
#include "dummy/dummy_helper_p.h"

QT_BEGIN_NAMESPACE

QLowEnergyControllerPrivateCommon::QLowEnergyControllerPrivateCommon()
    : QLowEnergyControllerPrivate()
{
    printDummyWarning();
    registerQLowEnergyControllerMetaType();
}

QLowEnergyControllerPrivateCommon::~QLowEnergyControllerPrivateCommon()
{
}

void QLowEnergyControllerPrivateCommon::init()
{
}

void QLowEnergyControllerPrivateCommon::connectToDevice()
{
    // required to pass unit test on default backend
    if (remoteDevice.isNull()) {
        qWarning() << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    qWarning() << "QLowEnergyControllerPrivateCommon::connectToDevice(): Not implemented";
    setError(QLowEnergyController::UnknownError);
}

void QLowEnergyControllerPrivateCommon::disconnectFromDevice()
{

}

void QLowEnergyControllerPrivateCommon::discoverServices()
{

}

void QLowEnergyControllerPrivateCommon::discoverServiceDetails(const QBluetoothUuid &/*service*/)
{

}

void QLowEnergyControllerPrivateCommon::readCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
                        const QLowEnergyHandle /*charHandle*/)
{

}

void QLowEnergyControllerPrivateCommon::readDescriptor(const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
                    const QLowEnergyHandle /*charHandle*/,
                    const QLowEnergyHandle /*descriptorHandle*/)
{

}

void QLowEnergyControllerPrivateCommon::writeCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
        const QLowEnergyHandle /*charHandle*/,
        const QByteArray &/*newValue*/,
        QLowEnergyService::WriteMode /*writeMode*/)
{

}

void QLowEnergyControllerPrivateCommon::writeDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
        const QLowEnergyHandle /*charHandle*/,
        const QLowEnergyHandle /*descriptorHandle*/,
        const QByteArray &/*newValue*/)
{

}

void QLowEnergyControllerPrivateCommon::startAdvertising(const QLowEnergyAdvertisingParameters &/* params */,
        const QLowEnergyAdvertisingData &/* advertisingData */,
        const QLowEnergyAdvertisingData &/* scanResponseData */)
{
}

void QLowEnergyControllerPrivateCommon::stopAdvertising()
{
}

void QLowEnergyControllerPrivateCommon::requestConnectionUpdate(const QLowEnergyConnectionParameters & /* params */)
{
}

void QLowEnergyControllerPrivateCommon::addToGenericAttributeList(const QLowEnergyServiceData &/* service */,
                                                            QLowEnergyHandle /* startHandle */)
{
}

QT_END_NAMESPACE
