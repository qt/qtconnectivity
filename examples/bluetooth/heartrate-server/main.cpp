// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QtCore/qbytearray.h>
#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

int main(int argc, char *argv[])
{
    // QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
#ifndef Q_OS_ANDROID
    QCoreApplication app(argc, argv);
#else
    QGuiApplication app(argc, argv);
#endif

    //! [Advertising Data]
    QLowEnergyAdvertisingData advertisingData;
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("HeartRateServer");
    advertisingData.setServices(QList<QBluetoothUuid>() << QBluetoothUuid::ServiceClassUuid::HeartRate);
    //! [Advertising Data]

    //! [Service Data]
    QLowEnergyCharacteristicData charData;
    charData.setUuid(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);
    charData.setValue(QByteArray(2, 0));
    charData.setProperties(QLowEnergyCharacteristic::Notify);
    const QLowEnergyDescriptorData clientConfig(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                QByteArray(2, 0));
    charData.addDescriptor(clientConfig);

    QLowEnergyServiceData serviceData;
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::HeartRate);
    serviceData.addCharacteristic(charData);
    //! [Service Data]

    //! [Start Advertising]
    bool errorOccurred = false;
    const QScopedPointer<QLowEnergyController> leController(QLowEnergyController::createPeripheral());
    auto errorHandler = [&leController,&errorOccurred](QLowEnergyController::Error errorCode)
    {
            qWarning().noquote().nospace() << errorCode << " occurred: "
                << leController->errorString();
            if (errorCode != QLowEnergyController::RemoteHostClosedError) {
                qWarning("Heartrate-server quitting due to the error.");
                errorOccurred = true;
                QCoreApplication::quit();
            }
    };
    QObject::connect(leController.data(), &QLowEnergyController::errorOccurred, errorHandler);

    QScopedPointer<QLowEnergyService> service(leController->addService(serviceData));
    leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData,
                                   advertisingData);
    if (errorOccurred)
        return -1;
    //! [Start Advertising]

    //! [Provide Heartbeat]
    QTimer heartbeatTimer;
    quint8 currentHeartRate = 60;
    enum ValueChange { ValueUp, ValueDown } valueChange = ValueUp;
    const auto heartbeatProvider = [&service, &currentHeartRate, &valueChange]() {
        QByteArray value;
        value.append(char(0)); // Flags that specify the format of the value.
        value.append(char(currentHeartRate)); // Actual value.
        QLowEnergyCharacteristic characteristic
                = service->characteristic(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);
        Q_ASSERT(characteristic.isValid());
        service->writeCharacteristic(characteristic, value); // Potentially causes notification.
        if (currentHeartRate == 60)
            valueChange = ValueUp;
        else if (currentHeartRate == 100)
            valueChange = ValueDown;
        if (valueChange == ValueUp)
            ++currentHeartRate;
        else
            --currentHeartRate;
    };
    QObject::connect(&heartbeatTimer, &QTimer::timeout, heartbeatProvider);
    heartbeatTimer.start(1000);
    //! [Provide Heartbeat]

    auto reconnect = [&leController, advertisingData, &service, serviceData]()
    {
        service.reset(leController->addService(serviceData));
        if (!service.isNull())
            leController->startAdvertising(QLowEnergyAdvertisingParameters(),
                                           advertisingData, advertisingData);
    };
    QObject::connect(leController.data(), &QLowEnergyController::disconnected, reconnect);

    const int retval = QCoreApplication::exec();
    return errorOccurred ? -1 : retval;
}
