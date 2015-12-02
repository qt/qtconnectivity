/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qhash.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qvector.h>

static QByteArray deviceName() { return "Qt GATT server"; }

static QScopedPointer<QLowEnergyController> leController;
typedef QSharedPointer<QLowEnergyService> ServicePtr;
static QHash<QBluetoothUuid, ServicePtr> services;

void addService(const QLowEnergyServiceData &serviceData)
{
    const ServicePtr service(leController->addService(serviceData));
    Q_ASSERT(service);
    services.insert(service->serviceUuid(), service);
}

void addRunningSpeedService()
{
    QLowEnergyServiceData serviceData;
    serviceData.setUuid(QBluetoothUuid::RunningSpeedAndCadence);
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);

    QLowEnergyDescriptorData desc;
    desc.setUuid(QBluetoothUuid::ClientCharacteristicConfiguration);
    desc.setValue(QByteArray(1, 0)); // Default: No indication, no notification.
    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid::RSCMeasurement);
    charData.addDescriptor(desc);
    charData.setProperties(QLowEnergyCharacteristic::Notify);
    QByteArray value(4, 0);
    value[0] = 1 << 2; // "Running", no optional fields.
    charData.setValue(value);
    serviceData.addCharacteristic(charData);

    charData = QLowEnergyCharacteristicData();
    charData.setUuid(QBluetoothUuid::RSCFeature);
    charData.setProperties(QLowEnergyCharacteristic::Read);
    value = QByteArray(2, 0);
    value[0] = 1 << 2; // "Walking or Running" supported.
    charData.setValue(value);
    serviceData.addCharacteristic(charData);
    addService(serviceData);
}

void addGenericAccessService()
{
    QLowEnergyServiceData serviceData;
    serviceData.setUuid(QBluetoothUuid::GenericAccess);
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);

    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid::DeviceName);
    charData.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Write);
    charData.setValue(deviceName());
    serviceData.addCharacteristic(charData);

    charData = QLowEnergyCharacteristicData();
    charData.setUuid(QBluetoothUuid::Appearance);
    charData.setProperties(QLowEnergyCharacteristic::Read);
    QByteArray value(2, 0);
    value[0] = 64; // Generic Phone
    charData.setValue(value);
    serviceData.addCharacteristic(charData);

    serviceData.addIncludedService(services.value(QBluetoothUuid::RunningSpeedAndCadence).data());
    addService(serviceData);
}

void addCustomService()
{
    QLowEnergyServiceData serviceData;
    serviceData.setUuid(QBluetoothUuid(quint16(0x2000))); // Made up.
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);

    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid(quint16(0x5000))); // Made up.
    charData.setProperties(QLowEnergyCharacteristic::Read);
    charData.setValue(QByteArray(1024, 'x')); // Long value to test "Read Blob".
    serviceData.addCharacteristic(charData);

    charData.setUuid(QBluetoothUuid(quint16(0x5001))); // Made up.
    charData.setProperties(QLowEnergyCharacteristic::Read);
    charData.setReadConstraints(QBluetooth::AttAuthorizationRequired); // To test read failure.
    serviceData.addCharacteristic(charData);
    charData.setValue("something");

    addService(serviceData);
}

void startAdvertising()
{
    QLowEnergyAdvertisingParameters params;
    params.setMode(QLowEnergyAdvertisingParameters::AdvInd);
    QLowEnergyAdvertisingData data;
    data.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityLimited);
    data.setServices(services.keys());
    data.setIncludePowerLevel(true);
    data.setLocalName(deviceName());
    leController->startAdvertising(params, data);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    leController.reset(QLowEnergyController::createPeripheral());
    addRunningSpeedService();
    addGenericAccessService();
    addCustomService();
    startAdvertising();

    // TODO: Change characteristics, client checks that it gets indication/notification
    // TODO: Where to test that we get the characteristicChanged signal for characteristics that the client writes?

    return app.exec();
}
