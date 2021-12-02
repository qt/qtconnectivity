/***************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QBluetoothLocalDevice>
#include <QLatin1String>
#include <QLoggingCategory>
#include <QLowEnergyAdvertisingData>
#include <QLowEnergyAdvertisingParameters>
#include <QLowEnergyCharacteristicData>
#include <QLowEnergyController>
#include <QLowEnergyDescriptorData>
#include <QLowEnergyServiceData>
#include <QLowEnergyDescriptorData>
#include <QTimer>

#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
#include <QGuiApplication>
#else
#include <QCoreApplication>
#endif // Q_OS_ANDROID || Q_OS_IOS

#include <thread>

static const QLatin1String largeCharacteristicServiceUuid("1f85e37c-ac16-11eb-ae5c-93d3a763feed");
static const QLatin1String largeCharacteristicCharUuid("40e4f68e-ac16-11eb-9956-cfe55a8c370c");

static const QLatin1String platformIdentifierServiceUuid("4a92cb7f-5031-4a09-8304-3e89413f458d");
static const QLatin1String platformIdentifierCharUuid("6b0ecf7c-5f09-4c87-aaab-bb49d5d383aa");

static const QLatin1String
        notificationIndicationTestServiceUuid("bb137ac5-5716-4b80-873b-e2d11d29efe2");
static const QLatin1String
        notificationIndicationTestChar1Uuid("6da4d652-0248-478a-a5a8-1e2f076158cc");
static const QLatin1String
        notificationIndicationTestChar2Uuid("990930f0-b9cc-4c27-8c1b-ebc2bcae5c95");
static const QLatin1String
        notificationIndicationTestChar3Uuid("9a60486b-de5b-4e03-b914-4e158c0bd388");
static const QLatin1String
        notificationIndicationTestChar4Uuid("d92435d4-6c2e-43f8-a6be-bbb66b5a3e28");

static const QLatin1String connectionCountServiceUuid("78c61a07-a0f9-4b92-be2d-2570d8dbf010");
static const QLatin1String connectionCountCharUuid("9414ec2d-792f-46a2-a19e-186d0fb38a08");

static const QLatin1String mtuServiceUuid("9a9483eb-cf4f-4c32-9a6b-794238d5b483");
static const QLatin1String mtuCharUuid("960d7e2a-a850-4a70-8064-cd74e9ccb6ff");

int main(int argc, char *argv[])
{
    qDebug() << "build:" << __DATE__ << __TIME__;
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    QGuiApplication app(argc, argv);
#else
    QCoreApplication app(argc, argv);
#endif // Q_OS_ANDROID || Q_OS_IOS

    // prepare list of services
    QList<QLowEnergyServiceData> serviceDefinitions;

    int connectioncount = 0;
    {
        // connection count service
        QLowEnergyServiceData serviceData;
        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid(connectionCountServiceUuid));

        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid(connectionCountCharUuid));
        size_t size = sizeof(int);
        QByteArray initialValue(size, 0);
        charData.setValue(initialValue);
        charData.setValueLength(size, size);
        charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read);

        serviceData.addCharacteristic(charData);

        serviceDefinitions << serviceData;
    }

    {
        // mtu size service
        QLowEnergyServiceData serviceData;
        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid(mtuServiceUuid));

        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid(mtuCharUuid));
        size_t size = sizeof(int);
        int mtu = 23;
        QByteArray initialValue((const char *)&mtu, sizeof(int));
        charData.setValue(initialValue);
        charData.setValueLength(size, size);
        charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                               | QLowEnergyCharacteristic::PropertyType::Notify);

        serviceData.addCharacteristic(charData);

        serviceDefinitions << serviceData;
    }
    {
        // large characteristic service
        //
        // This is just a service offering a 512 bytes large characteristic which can
        // be read and written. It is used to test reading and writing at MTU sizes smaller
        // than the size of the characteristic.
        QLowEnergyServiceData serviceData;
        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid(largeCharacteristicServiceUuid));

        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid(largeCharacteristicCharUuid));
        QByteArray initialValue(512, 0);
        initialValue[0] = 0x0b;
        charData.setValue(initialValue);
        charData.setValueLength(512, 512);
        charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                               | QLowEnergyCharacteristic::PropertyType::Write);

        serviceData.addCharacteristic(charData);

        serviceDefinitions << serviceData;
    }

    {

        // notification service
        //
        // This is a service which offers:
        //   - one characteristic which does not support notification or indication
        //   - one characteristic which does only support notification
        //   - one characteristic which does only support indication
        //   - one characteristic which supports both notification or indication
        // to test their discovery
        QLowEnergyServiceData serviceData;

        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid(notificationIndicationTestServiceUuid));

        {
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(notificationIndicationTestChar1Uuid));
            QByteArray initialValue(8, 0);
            initialValue[0] = 0x0b;
            charData.setValue(initialValue);
            charData.setValueLength(8, 8);
            charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read);

            serviceData.addCharacteristic(charData);
        }
        {
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(notificationIndicationTestChar2Uuid));
            QByteArray initialValue(8, 0);
            initialValue[0] = 0x0b;
            charData.setValue(initialValue);
            charData.setValueLength(8, 8);
            charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                                   | QLowEnergyCharacteristic::PropertyType::Notify);

            const QLowEnergyDescriptorData clientConfig(
                    QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                    QLowEnergyCharacteristic::CCCDDisable);
            charData.addDescriptor(clientConfig);

            serviceData.addCharacteristic(charData);
        }
        {
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(notificationIndicationTestChar3Uuid));
            QByteArray initialValue(8, 0);
            initialValue[0] = 0x0b;
            charData.setValue(initialValue);
            charData.setValueLength(8, 8);
            charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                                   | QLowEnergyCharacteristic::PropertyType::Indicate);

            const QLowEnergyDescriptorData clientConfig(
                    QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                    QLowEnergyCharacteristic::CCCDDisable);
            charData.addDescriptor(clientConfig);

            serviceData.addCharacteristic(charData);
        }
        {
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(notificationIndicationTestChar4Uuid));
            QByteArray initialValue(8, 0);
            initialValue[0] = 0x0b;
            charData.setValue(initialValue);
            charData.setValueLength(8, 8);
            charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read
                                   | QLowEnergyCharacteristic::PropertyType::Notify
                                   | QLowEnergyCharacteristic::PropertyType::Indicate);

            const QLowEnergyDescriptorData clientConfig(
                    QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                    QLowEnergyCharacteristic::CCCDDisable);
            charData.addDescriptor(clientConfig);

            serviceData.addCharacteristic(charData);
        }

        serviceDefinitions << serviceData;
    }

    {
        // server's platform identifier service
        QLowEnergyServiceData serviceData;
        serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceData.setUuid(QBluetoothUuid(platformIdentifierServiceUuid));

        QLowEnergyCharacteristicData charData;
        charData.setUuid(QBluetoothUuid(platformIdentifierCharUuid));
#if defined(Q_OS_ANDROID)
        QByteArray platformIdentifier("android");
#elif defined(QT_OS_WIN)
        QByteArray platformIdentifier("windows");
#elif defined(Q_OS_DARWIN)
        QByteArray platformIdentifier("darwin");
#elif defined(Q_OS_LINUX)
        QByteArray platformIdentifier("linux");
#else
        QByteArray platformIdentifier("unspecified");
#endif
        qDebug() << "Server will report it is running on: " << platformIdentifier;
        charData.setValue(platformIdentifier);
        charData.setProperties(QLowEnergyCharacteristic::PropertyType::Read);

        serviceData.addCharacteristic(charData);

        serviceDefinitions << serviceData;
    }

#ifndef Q_OS_IOS
    auto localAdapters = QBluetoothLocalDevice::allDevices();
    QBluetoothLocalDevice adapter(localAdapters.back().address());
    adapter.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
#endif // Q_OS_IOS

    // Advertising data
    QLowEnergyAdvertisingData advertisingData;
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("BluetoothTestDevice");
    QList<QBluetoothUuid> serviceUuids;
    for (const auto &serviceData : serviceDefinitions) {
        serviceUuids << serviceData.uuid();
    }
    // leads to too large advertising data
    // advertisingData.setServices(serviceUuids);

    // start advertising
#ifndef Q_OS_IOS
    auto devices = QBluetoothLocalDevice::allDevices();
    qDebug() << "advertising on" << devices.back().address();
    const QScopedPointer<QLowEnergyController> leController(
            QLowEnergyController::createPeripheral(devices.back().address()));
#else
    const QScopedPointer<QLowEnergyController> leController(
            QLowEnergyController::createPeripheral());
#endif // Q_OS_IOS
    QList<QSharedPointer<QLowEnergyService>> services;

    for (const auto &serviceData : serviceDefinitions) {
        services.emplaceBack(leController->addService(serviceData));
    }

    leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData,
                                   advertisingData);


    auto reconnect = [&connectioncount, &leController, advertisingData, &services, serviceDefinitions]() {
        connectioncount++;
        for (int i = 0; i < services.size(); ++i) {
            services[i].reset(leController->addService(serviceDefinitions[i]));
        }


        {
            // set connection counter
            Q_ASSERT(services[0]->serviceUuid()
                     == QBluetoothUuid(connectionCountServiceUuid));
            QByteArray value((const char *)&connectioncount, sizeof(int));
            QLowEnergyCharacteristic characteristic = services[0]->characteristic(
                    QBluetoothUuid(connectionCountCharUuid));
            Q_ASSERT(characteristic.isValid());
            services[0]->writeCharacteristic(characteristic, value);
        }

        bool allValid = true;
        for (const auto &service : services) {
            allValid = allValid & !service.isNull();
        }

        if (allValid){
            leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData,
                                           advertisingData);
            qDebug() << "starting advertising for " << connectioncount << "th time.";
        }
        else
            qDebug() << "Cannot start advertising: Service not valid.";
    };
    QObject::connect(leController.data(), &QLowEnergyController::disconnected, reconnect);

    QObject::connect(leController.data(), &QLowEnergyController::mtuChanged, [&services](int mtu) {
        qDebug() << "MTU changed, callback called.";
        Q_ASSERT(services[1]->serviceUuid() == QBluetoothUuid(mtuServiceUuid));
        QByteArray value((const char *)&mtu, sizeof(int));
        QLowEnergyCharacteristic characteristic =
                services[1]->characteristic(QBluetoothUuid(mtuCharUuid));
        Q_ASSERT(characteristic.isValid());
        services[1]->writeCharacteristic(characteristic, value);
    });

    QTimer notificationTestTimer;
    quint64 currentCharacteristicValue = 0;
    const auto characteristicChanger = [&services, &currentCharacteristicValue]() {
        QByteArray value((const char *)&currentCharacteristicValue, 8);

        Q_ASSERT(services[3]->serviceUuid()
                 == QBluetoothUuid(notificationIndicationTestServiceUuid));
        {
            QLowEnergyCharacteristic characteristic = services[3]->characteristic(
                    QBluetoothUuid(notificationIndicationTestChar2Uuid));
            Q_ASSERT(characteristic.isValid());
            services[3]->writeCharacteristic(characteristic,
                                             value); // Potentially causes notification.
        }
        {
            QLowEnergyCharacteristic characteristic = services[3]->characteristic(
                    QBluetoothUuid(notificationIndicationTestChar3Uuid));
            Q_ASSERT(characteristic.isValid());
            services[3]->writeCharacteristic(characteristic,
                                             value); // Potentially causes notification.
        }
        {
            QLowEnergyCharacteristic characteristic = services[3]->characteristic(
                    QBluetoothUuid(notificationIndicationTestChar4Uuid));
            Q_ASSERT(characteristic.isValid());
            services[3]->writeCharacteristic(characteristic,
                                             value); // Potentially causes notification.
        }

        ++currentCharacteristicValue;
    };
    QObject::connect(&notificationTestTimer, &QTimer::timeout, characteristicChanger);
    notificationTestTimer.start(100);

    return app.exec();
}
