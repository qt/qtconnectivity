/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qlowenergycontroller_p.h"
#include <QtCore/QLoggingCategory>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtBluetooth/QLowEnergyServiceData>
#include <QtBluetooth/QLowEnergyCharacteristicData>
#include <QtBluetooth/QLowEnergyDescriptorData>
#include <QtBluetooth/QLowEnergyAdvertisingData>
#include <QtBluetooth/QLowEnergyAdvertisingParameters>
#include <QtBluetooth/QLowEnergyConnectionParameters>


QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

QLowEnergyControllerPrivate::QLowEnergyControllerPrivate()
    : QObject(),
      state(QLowEnergyController::UnconnectedState),
      error(QLowEnergyController::NoError),
      hub(0)
{
    registerQLowEnergyControllerMetaType();
}

QLowEnergyControllerPrivate::~QLowEnergyControllerPrivate()
{
    if (role == QLowEnergyController::PeripheralRole) {
        if (hub)
            hub->javaObject().callMethod<void>("disconnectServer");
    }
}

void QLowEnergyControllerPrivate::init()
{
    // Android Central/Client support starts with v18
    // Peripheral/Server support requires Android API v21
    const bool isPeripheral = (role == QLowEnergyController::PeripheralRole);
    const jint version = QtAndroidPrivate::androidSdkVersion();

    if (isPeripheral) {
        if (version < 21) {
            qWarning() << "Qt Bluetooth LE Peripheral support not available"
                          "on Android devices below version 21";
            return;
        }

        hub = new LowEnergyNotificationHub(remoteDevice, isPeripheral, this);
        // we only connect to the peripheral role specific signals
        // TODO add connections as they get added later on
    } else {
        if (version < 18) {
            qWarning() << "Qt Bluetooth LE Central/Client support not available"
                          "on Android devices below version 18";
            return;
        }

        hub = new LowEnergyNotificationHub(remoteDevice, isPeripheral, this);
        // we only connect to the central role specific signals
        connect(hub, &LowEnergyNotificationHub::connectionUpdated,
                this, &QLowEnergyControllerPrivate::connectionUpdated);
        connect(hub, &LowEnergyNotificationHub::servicesDiscovered,
                this, &QLowEnergyControllerPrivate::servicesDiscovered);
        connect(hub, &LowEnergyNotificationHub::serviceDetailsDiscoveryFinished,
                this, &QLowEnergyControllerPrivate::serviceDetailsDiscoveryFinished);
        connect(hub, &LowEnergyNotificationHub::characteristicRead,
                this, &QLowEnergyControllerPrivate::characteristicRead);
        connect(hub, &LowEnergyNotificationHub::descriptorRead,
                this, &QLowEnergyControllerPrivate::descriptorRead);
        connect(hub, &LowEnergyNotificationHub::characteristicWritten,
                this, &QLowEnergyControllerPrivate::characteristicWritten);
        connect(hub, &LowEnergyNotificationHub::descriptorWritten,
                this, &QLowEnergyControllerPrivate::descriptorWritten);
        connect(hub, &LowEnergyNotificationHub::characteristicChanged,
                this, &QLowEnergyControllerPrivate::characteristicChanged);
        connect(hub, &LowEnergyNotificationHub::serviceError,
                this, &QLowEnergyControllerPrivate::serviceError);
    }
}

void QLowEnergyControllerPrivate::connectToDevice()
{
    if (!hub)
        return; // Android version below v18

    // required to pass unit test on default backend
    if (remoteDevice.isNull()) {
        qWarning() << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    setState(QLowEnergyController::ConnectingState);

    if (!hub->javaObject().isValid()) {
        qCWarning(QT_BT_ANDROID) << "Cannot initiate QtBluetoothLE";
        setError(QLowEnergyController::ConnectionError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }

    bool result = hub->javaObject().callMethod<jboolean>("connect");
    if (!result) {
        setError(QLowEnergyController::ConnectionError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }
}

void QLowEnergyControllerPrivate::disconnectFromDevice()
{
    /* Catch an Android timeout bug. If the device is connecting but cannot
     * physically connect it seems to ignore the disconnect call below.
     * At least BluetoothGattCallback.onConnectionStateChange never
     * arrives. The next BluetoothGatt.connect() works just fine though.
     * */

    QLowEnergyController::ControllerState oldState = state;
    setState(QLowEnergyController::ClosingState);

    if (hub)
        hub->javaObject().callMethod<void>("disconnect");

    if (oldState == QLowEnergyController::ConnectingState)
        setState(QLowEnergyController::UnconnectedState);
}

void QLowEnergyControllerPrivate::discoverServices()
{
    if (hub && hub->javaObject().callMethod<jboolean>("discoverServices")) {
        qCDebug(QT_BT_ANDROID) << "Service discovery initiated";
    } else {
        //revert to connected state
        setError(QLowEnergyController::NetworkError);
        setState(QLowEnergyController::ConnectedState);
    }
}

void QLowEnergyControllerPrivate::discoverServiceDetails(const QBluetoothUuid &service)
{
    if (!serviceList.contains(service)) {
        qCWarning(QT_BT_ANDROID) << "Discovery of unknown service" << service.toString()
                                 << "not possible";
        return;
    }

    if (!hub)
        return;

    //cut leading { and trailing } {xxx-xxx}
    QString tempUuid = service.toString();
    tempUuid.chop(1); //remove trailing '}'
    tempUuid.remove(0, 1); //remove first '{'

    QAndroidJniEnvironment env;
    QAndroidJniObject uuid = QAndroidJniObject::fromString(tempUuid);
    bool result = hub->javaObject().callMethod<jboolean>("discoverServiceDetails",
                                                         "(Ljava/lang/String;)Z",
                                                         uuid.object<jstring>());
    if (!result) {
        QSharedPointer<QLowEnergyServicePrivate> servicePrivate =
                serviceList.value(service);
        if (!servicePrivate.isNull()) {
            servicePrivate->setError(QLowEnergyService::UnknownError);
            servicePrivate->setState(QLowEnergyService::DiscoveryRequired);
        }
        qCWarning(QT_BT_ANDROID) << "Cannot discover details for" << service.toString();
        return;
    }

    qCDebug(QT_BT_ANDROID) << "Discovery of" << service << "started";
}

void QLowEnergyControllerPrivate::writeCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QByteArray &newValue,
        QLowEnergyService::WriteMode mode)
{
    //TODO don't ignore WriteWithResponse, right now we assume responses
    Q_ASSERT(!service.isNull());

    if (!service->characteristicList.contains(charHandle))
        return;

    QAndroidJniEnvironment env;
    jbyteArray payload;
    payload = env->NewByteArray(newValue.size());
    env->SetByteArrayRegion(payload, 0, newValue.size(),
                            (jbyte *)newValue.constData());

    bool result = false;
    if (hub) {
        qCDebug(QT_BT_ANDROID) << "Write characteristic with handle " << charHandle
                 << newValue.toHex() << "(service:" << service->uuid
                 << ", writeWithResponse:" << (mode == QLowEnergyService::WriteWithResponse)
                 << ", signed:" << (mode == QLowEnergyService::WriteSigned) << ")";
        result = hub->javaObject().callMethod<jboolean>("writeCharacteristic", "(I[BI)Z",
                      charHandle, payload, mode);
    }

    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        result = false;
    }

    env->DeleteLocalRef(payload);

    if (!result)
        service->setError(QLowEnergyService::CharacteristicWriteError);
}

void QLowEnergyControllerPrivate::writeDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle /*charHandle*/,
        const QLowEnergyHandle descHandle,
        const QByteArray &newValue)
{
    Q_ASSERT(!service.isNull());

    QAndroidJniEnvironment env;
    jbyteArray payload;
    payload = env->NewByteArray(newValue.size());
    env->SetByteArrayRegion(payload, 0, newValue.size(),
                            (jbyte *)newValue.constData());

    bool result = false;
    if (hub) {
        qCDebug(QT_BT_ANDROID) << "Write descriptor with handle " << descHandle
                 << newValue.toHex() << "(service:" << service->uuid << ")";
        result = hub->javaObject().callMethod<jboolean>("writeDescriptor", "(I[B)Z",
                                                        descHandle, payload);
    }

    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        result = false;
    }

    env->DeleteLocalRef(payload);

    if (!result)
        service->setError(QLowEnergyService::DescriptorWriteError);
}

void QLowEnergyControllerPrivate::readCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle)
{
    Q_ASSERT(!service.isNull());

    if (!service->characteristicList.contains(charHandle))
        return;

    QAndroidJniEnvironment env;
    bool result = false;
    if (hub) {
        qCDebug(QT_BT_ANDROID) << "Read characteristic with handle"
                               <<  charHandle << service->uuid;
        result = hub->javaObject().callMethod<jboolean>("readCharacteristic",
                      "(I)Z", charHandle);
    }

    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        result = false;
    }

    if (!result)
        service->setError(QLowEnergyService::CharacteristicReadError);
}

void QLowEnergyControllerPrivate::readDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle /*charHandle*/,
        const QLowEnergyHandle descriptorHandle)
{
    Q_ASSERT(!service.isNull());

    QAndroidJniEnvironment env;
    bool result = false;
    if (hub) {
        qCDebug(QT_BT_ANDROID) << "Read descriptor with handle"
                               <<  descriptorHandle << service->uuid;
        result = hub->javaObject().callMethod<jboolean>("readDescriptor",
                      "(I)Z", descriptorHandle);
    }

    if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        result = false;
    }

    if (!result)
        service->setError(QLowEnergyService::DescriptorReadError);
}

void QLowEnergyControllerPrivate::connectionUpdated(
        QLowEnergyController::ControllerState newState,
        QLowEnergyController::Error errorCode)
{
    Q_Q(QLowEnergyController);

    const QLowEnergyController::ControllerState oldState = state;
    qCDebug(QT_BT_ANDROID) << "Connection updated:"
                           << "error:" << errorCode
                           << "oldState:" << oldState
                           << "newState:" << newState;

    if (errorCode != QLowEnergyController::NoError) {
        // ConnectionError if transition from Connecting to Connected
        if (oldState == QLowEnergyController::ConnectingState) {
            setError(QLowEnergyController::ConnectionError);
            /* There is a bug in Android, when connecting to an unconnectable
             * device. The connection times out and Android sends error code
             * 133 (doesn't exist) and STATE_CONNECTED. A subsequent disconnect()
             * call never sends a STATE_DISCONNECTED either.
             * As workaround we will trigger disconnect when we encounter
             * error during connect attempt. This leaves the controller
             * in a cleaner state.
             * */
            newState = QLowEnergyController::UnconnectedState;
        }
        else
            setError(errorCode);
    }

    setState(newState);
    if (newState == QLowEnergyController::UnconnectedState
            && !(oldState == QLowEnergyController::UnconnectedState
                || oldState == QLowEnergyController::ConnectingState)) {

        // Invalidate the services if the disconnect came from the remote end.
        // Qtherwise we disconnected via QLowEnergyController::disconnectDevice() which
        // triggered invalidation already
        if (!serviceList.isEmpty()) {
            Q_ASSERT(oldState != QLowEnergyController::ClosingState);
            invalidateServices();
        }
        emit q->disconnected();
    } else if (newState == QLowEnergyController::ConnectedState
               && oldState != QLowEnergyController::ConnectedState ) {
        emit q->connected();
    }
}

void QLowEnergyControllerPrivate::servicesDiscovered(
        QLowEnergyController::Error errorCode, const QString &foundServices)
{
    Q_Q(QLowEnergyController);

    if (errorCode == QLowEnergyController::NoError) {
        //Android delivers all services in one go
        const QStringList list = foundServices.split(QStringLiteral(" "), QString::SkipEmptyParts);
        foreach (const QString &entry, list) {
            const QBluetoothUuid service(entry);
            if (service.isNull())
                return;

            QLowEnergyServicePrivate *priv = new QLowEnergyServicePrivate();
            priv->uuid = service;
            priv->setController(this);

            QSharedPointer<QLowEnergyServicePrivate> pointer(priv);
            serviceList.insert(service, pointer);

            emit q->serviceDiscovered(QBluetoothUuid(entry));
        }

        setState(QLowEnergyController::DiscoveredState);
        emit q->discoveryFinished();
    } else {
        setError(errorCode);
        setState(QLowEnergyController::ConnectedState);
    }
}

void QLowEnergyControllerPrivate::serviceDetailsDiscoveryFinished(
        const QString &serviceUuid, int startHandle, int endHandle)
{
    const QBluetoothUuid service(serviceUuid);
    if (!serviceList.contains(service)) {
        qCWarning(QT_BT_ANDROID) << "Discovery done of unknown service:"
                                 << service.toString();
        return;
    }

    //update service data
    QSharedPointer<QLowEnergyServicePrivate> pointer =
            serviceList.value(service);
    pointer->startHandle = startHandle;
    pointer->endHandle = endHandle;

    if (hub && hub->javaObject().isValid()) {
        QAndroidJniObject uuid = QAndroidJniObject::fromString(serviceUuid);
        QAndroidJniObject javaIncludes = hub->javaObject().callObjectMethod(
                                        "includedServices",
                                        "(Ljava/lang/String;)Ljava/lang/String;",
                                        uuid.object<jstring>());
        if (javaIncludes.isValid()) {
            const QStringList list = javaIncludes.toString()
                                                 .split(QStringLiteral(" "),
                                                        QString::SkipEmptyParts);
            foreach (const QString &entry, list) {
                const QBluetoothUuid service(entry);
                if (service.isNull())
                    return;

                pointer->includedServices.append(service);

                // update the type of the included service
                QSharedPointer<QLowEnergyServicePrivate> otherService =
                        serviceList.value(service);
                if (!otherService.isNull())
                    otherService->type |= QLowEnergyService::IncludedService;
            }
        }
    }

    qCDebug(QT_BT_ANDROID) << "Service" << serviceUuid << "discovered (start:"
              << startHandle << "end:" << endHandle << ")" << pointer.data();

    pointer->setState(QLowEnergyService::ServiceDiscovered);
}

void QLowEnergyControllerPrivate::characteristicRead(
        const QBluetoothUuid &serviceUuid, int handle,
        const QBluetoothUuid &charUuid, int properties, const QByteArray &data)
{
    if (!serviceList.contains(serviceUuid))
        return;

    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceList.value(serviceUuid);
    QLowEnergyHandle charHandle = handle;

    QLowEnergyServicePrivate::CharData &charDetails =
            service->characteristicList[charHandle];

    //Android uses same property value as Qt which is the Bluetooth LE standard
    charDetails.properties = QLowEnergyCharacteristic::PropertyType(properties);
    charDetails.uuid = charUuid;
    charDetails.value = data;
    //value handle always one larger than characteristics value handle
    charDetails.valueHandle = charHandle + 1;

    if (service->state == QLowEnergyService::ServiceDiscovered) {
        QLowEnergyCharacteristic characteristic = characteristicForHandle(charHandle);
        if (!characteristic.isValid()) {
            qCWarning(QT_BT_ANDROID) << "characteristicRead: Cannot find characteristic";
            return;
        }
        emit service->characteristicRead(characteristic, data);
    }
}

void QLowEnergyControllerPrivate::descriptorRead(
        const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid,
        int descHandle, const QBluetoothUuid &descUuid, const QByteArray &data)
{
    if (!serviceList.contains(serviceUuid))
        return;

    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceList.value(serviceUuid);

    bool entryUpdated = false;

    CharacteristicDataMap::iterator charIt = service->characteristicList.begin();
    for ( ; charIt != service->characteristicList.end(); ++charIt) {
        QLowEnergyServicePrivate::CharData &charDetails = charIt.value();

        if (charDetails.uuid != charUuid)
            continue;

        // new entry created if it doesn't exist
        QLowEnergyServicePrivate::DescData &descDetails =
                charDetails.descriptorList[descHandle];
        descDetails.uuid = descUuid;
        descDetails.value = data;
        entryUpdated = true;
        break;
    }

    if (!entryUpdated) {
        qCWarning(QT_BT_ANDROID) << "Cannot find/update descriptor"
                                 << descUuid << charUuid << serviceUuid;
    } else if (service->state == QLowEnergyService::ServiceDiscovered){
        QLowEnergyDescriptor descriptor = descriptorForHandle(descHandle);
        if (!descriptor.isValid()) {
            qCWarning(QT_BT_ANDROID) << "descriptorRead: Cannot find descriptor";
            return;
        }
        emit service->descriptorRead(descriptor, data);
    }
}

void QLowEnergyControllerPrivate::characteristicWritten(
        int charHandle, const QByteArray &data, QLowEnergyService::ServiceError errorCode)
{
    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceForHandle(charHandle);
    if (service.isNull())
        return;

    qCDebug(QT_BT_ANDROID) << "Characteristic write confirmation" << service->uuid
                           << charHandle << data.toHex() << errorCode;

    if (errorCode != QLowEnergyService::NoError) {
        service->setError(errorCode);
        return;
    }

    QLowEnergyCharacteristic characteristic = characteristicForHandle(charHandle);
    if (!characteristic.isValid()) {
        qCWarning(QT_BT_ANDROID) << "characteristicWritten: Cannot find characteristic";
        return;
    }

    // only update cache when property is readable. Otherwise it remains
    // empty.
    if (characteristic.properties() & QLowEnergyCharacteristic::Read)
        updateValueOfCharacteristic(charHandle, data, false);
    emit service->characteristicWritten(characteristic, data);
}

void QLowEnergyControllerPrivate::descriptorWritten(
        int descHandle, const QByteArray &data, QLowEnergyService::ServiceError errorCode)
{
    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceForHandle(descHandle);
    if (service.isNull())
        return;

    qCDebug(QT_BT_ANDROID) << "Descriptor write confirmation" << service->uuid
                           << descHandle << data.toHex() << errorCode;

    if (errorCode != QLowEnergyService::NoError) {
        service->setError(errorCode);
        return;
    }

    QLowEnergyDescriptor descriptor = descriptorForHandle(descHandle);
    if (!descriptor.isValid()) {
        qCWarning(QT_BT_ANDROID) << "descriptorWritten: Cannot find descriptor";
        return;
    }

    updateValueOfDescriptor(descriptor.characteristicHandle(),
                            descHandle, data, false);
    emit service->descriptorWritten(descriptor, data);
}

void QLowEnergyControllerPrivate::characteristicChanged(
        int charHandle, const QByteArray &data)
{
    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceForHandle(charHandle);
    if (service.isNull())
        return;

    qCDebug(QT_BT_ANDROID) << "Characteristic change notification" << service->uuid
                           << charHandle << data.toHex();

    QLowEnergyCharacteristic characteristic = characteristicForHandle(charHandle);
    if (!characteristic.isValid()) {
        qCWarning(QT_BT_ANDROID) << "characteristicChanged: Cannot find characteristic";
        return;
    }

    // only update cache when property is readable. Otherwise it remains
    // empty.
    if (characteristic.properties() & QLowEnergyCharacteristic::Read)
        updateValueOfCharacteristic(characteristic.attributeHandle(),
                                data, false);
    emit service->characteristicChanged(characteristic, data);
}

void QLowEnergyControllerPrivate::serviceError(
        int attributeHandle, QLowEnergyService::ServiceError errorCode)
{
    // ignore call if it isn't really an error
    if (errorCode == QLowEnergyService::NoError)
        return;

    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceForHandle(attributeHandle);
    Q_ASSERT(!service.isNull());

    // ATM we don't really use attributeHandle but later on we might
    // want to associate the error code with a char or desc
    service->setError(errorCode);
}

static QAndroidJniObject javaParcelUuidfromQtUuid(const QBluetoothUuid& uuid)
{
    QString output = uuid.toString();
    // cut off leading and trailing brackets
    output = output.mid(1, output.size()-2);

    QAndroidJniObject javaString = QAndroidJniObject::fromString(output);
    QAndroidJniObject parcelUuid = QAndroidJniObject::callStaticObjectMethod(
                "android/os/ParcelUuid", "fromString",
                "(Ljava/lang/String;)Landroid/os/ParcelUuid;", javaString.object());

    return parcelUuid;
}

static QAndroidJniObject createJavaAdvertiseData(const QLowEnergyAdvertisingData &data)
{
    QAndroidJniObject builder = QAndroidJniObject("android/bluetooth/le/AdvertiseData$Builder");

    // device name cannot be set but there is choice to show it or not
    builder = builder.callObjectMethod("setIncludeDeviceName", "(Z)Landroid/bluetooth/le/AdvertiseData$Builder;",
                                       !data.localName().isEmpty());
    builder = builder.callObjectMethod("setIncludeTxPowerLevel", "(Z)Landroid/bluetooth/le/AdvertiseData$Builder;",
                                       data.includePowerLevel());
    for (const auto service: data.services())
    {
        builder = builder.callObjectMethod("addServiceUuid",
                                       "(Landroid/os/ParcelUuid;)Landroid/bluetooth/le/AdvertiseData$Builder;",
                                       javaParcelUuidfromQtUuid(service).object());
    }

    if (!data.manufacturerData().isEmpty()) {
        QAndroidJniEnvironment env;
        const qint32 nativeSize = data.manufacturerData().size();
        jbyteArray nativeData = env->NewByteArray(nativeSize);
        env->SetByteArrayRegion(nativeData, 0, nativeSize,
                                reinterpret_cast<const jbyte*>(data.manufacturerData().constData()));
        builder = builder.callObjectMethod("addManufacturerData",
                                       "(I[B])Landroid/bluetooth/le/AdvertiseData$Builder;",
                                       data.manufacturerId(), nativeData);
        env->DeleteLocalRef(nativeData);

        if (env->ExceptionCheck()) {
            qCWarning(QT_BT_ANDROID) << "Cannot set manufacturer id/data";
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }

    /*// TODO Qt vs Java API mismatch
          -> Qt assumes rawData() is a global field
          -> Android pairs rawData() per service uuid
    if (!data.rawData().isEmpty()) {
        QAndroidJniEnvironment env;
        qint32 nativeSize = data.rawData().size();
        jbyteArray nativeData = env->NewByteArray(nativeSize);
        env->SetByteArrayRegion(nativeData, 0, nativeSize,
                                reinterpret_cast<const jbyte*>(data.rawData().constData()));
        builder = builder.callObjectMethod("addServiceData",
                                       "(Landroid/os/ParcelUuid;[B])Landroid/bluetooth/le/AdvertiseData$Builder;",
                                       data.rawData().object(), nativeData);
        env->DeleteLocalRef(nativeData);

        if (env->ExceptionCheck()) {
            qCWarning(QT_BT_ANDROID) << "Cannot set advertisement raw data";
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }*/

    QAndroidJniObject javaAdvertiseData = builder.callObjectMethod("build",
                                       "()Landroid/bluetooth/le/AdvertiseData;");
    return javaAdvertiseData;
}

static QAndroidJniObject createJavaAdvertiseSettings(const QLowEnergyAdvertisingParameters& params)
{
    QAndroidJniObject builder = QAndroidJniObject("android/bluetooth/le/AdvertiseSettings$Builder");

    bool connectable = false;
    switch (params.mode())
    {
    case QLowEnergyAdvertisingParameters::AdvInd:
        connectable = true;
        break;
    case QLowEnergyAdvertisingParameters::AdvScanInd:
    case QLowEnergyAdvertisingParameters::AdvNonConnInd:
        connectable = false;
        break;
    // intentionally no default case
    }
    builder = builder.callObjectMethod("setConnectable", "(Z)Landroid/bluetooth/le/AdvertiseSettings$Builder;",
                                       connectable);

    /* TODO No Android API for further QLowEnergyAdvertisingParameters options
     *      Android TxPowerLevel, AdvertiseMode and Timeout not mappable to Qt
     */

    QAndroidJniObject javaAdvertiseSettings = builder.callObjectMethod("build",
                                            "()Landroid/bluetooth/le/AdvertiseSettings;");
    return javaAdvertiseSettings;
}


void QLowEnergyControllerPrivate::startAdvertising(const QLowEnergyAdvertisingParameters &params,
        const QLowEnergyAdvertisingData &advertisingData,
        const QLowEnergyAdvertisingData &scanResponseData)
{
    setState(QLowEnergyController::AdvertisingState);

    if (!hub->javaObject().isValid()) {
        qCWarning(QT_BT_ANDROID) << "Cannot initiate QtBluetoothLEServer";
        setError(QLowEnergyController::AdvertisingError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }

    // Pass on advertisingData, scanResponse & AdvertiseSettings
    QAndroidJniObject jAdvertiseData = createJavaAdvertiseData(advertisingData);
    QAndroidJniObject jScanResponse = createJavaAdvertiseData(scanResponseData);
    QAndroidJniObject jAdvertiseSettings = createJavaAdvertiseSettings(params);

    const bool result = hub->javaObject().callMethod<jboolean>("startAdvertising",
            "(Landroid/bluetooth/le/AdvertiseData;Landroid/bluetooth/le/AdvertiseData;Landroid/bluetooth/le/AdvertiseSettings;)Z",
            jAdvertiseData.object(), jScanResponse.object(), jAdvertiseSettings.object());
    if (!result) {
        setError(QLowEnergyController::AdvertisingError);
        setState(QLowEnergyController::UnconnectedState);
    }
}

void QLowEnergyControllerPrivate::stopAdvertising()
{
    setState(QLowEnergyController::UnconnectedState);
    hub->javaObject().callMethod<void>("stopAdvertising");
}

void QLowEnergyControllerPrivate::requestConnectionUpdate(const QLowEnergyConnectionParameters &params)
{
    // Possible since Android v21
    // Android does not permit specification of specific latency or min/max
    // connection intervals (see BluetoothGatt.requestConnectionPriority()
    // In fact, each device manufacturer is permitted to change those values via a config
    // file too. Therefore we can only make an approximated guess (see implementation below)
    // In addition there is no feedback signal (known bug) from the hardware layer as per v24.

    // TODO recheck in later Android releases whether callback for
    // BluetoothGatt.requestConnectionPriority() was added

    if (role != QLowEnergyController::CentralRole) {
        qCWarning(QT_BT_ANDROID) << "On Android, connection requests only work for central role";
        return;
    }

    bool result = hub->javaObject().callMethod<jboolean>("requestConnectionUpdatePriority",
                                                         "(D)Z", params.minimumInterval());
    if (!result)
        qCWarning(QT_BT_ANDROID) << "Cannot set connection update priority";
}

// Conversion: QBluetoothUuid -> java.util.UUID
static QAndroidJniObject javaUuidfromQtUuid(const QBluetoothUuid& uuid)
{
    QString output = uuid.toString();
    // cut off leading and trailing brackets
    output = output.mid(1, output.size()-2);

    QAndroidJniObject javaString = QAndroidJniObject::fromString(output);
    QAndroidJniObject javaUuid = QAndroidJniObject::callStaticObjectMethod(
                "java/util/UUID", "fromString", "(Ljava/lang/String;)Ljava/util/UUID;",
                javaString.object());

    return javaUuid;
}

void QLowEnergyControllerPrivate::addToGenericAttributeList(const QLowEnergyServiceData &serviceData,
                                                            QLowEnergyHandle startHandle)
{
    QSharedPointer<QLowEnergyServicePrivate> service = serviceForHandle(startHandle);
    if (service.isNull())
        return;

    // create BluetoothGattService object
    jint sType = QAndroidJniObject::getStaticField<jint>(
                            "android/bluetooth/BluetoothGattService", "SERVICE_TYPE_PRIMARY");
    if (serviceData.type() == QLowEnergyServiceData::ServiceTypeSecondary)
        sType = QAndroidJniObject::getStaticField<jint>(
                                    "android/bluetooth/BluetoothGattService", "SERVICE_TYPE_SECONDARY");

    service->androidService = QAndroidJniObject("android/bluetooth/BluetoothGattService",
                                                "(Ljava/util/UUID;I)V",
                                                javaUuidfromQtUuid(service->uuid).object(), sType);

    // add included services, which must have been added earlier already
    const QList<QLowEnergyService*> includedServices = serviceData.includedServices();
    for (const auto includedServiceEntry: includedServices) {
        //TODO test this end-to-end
        const jboolean result = service->androidService.callMethod<jboolean>(
                        "addService", "(Landroid/bluetooth/BluetoothGattService;)Z",
                        includedServiceEntry->d_ptr->androidService.object());
        if (!result)
            qWarning(QT_BT_ANDROID) << "Cannot add included service " << includedServiceEntry->serviceUuid()
                                    << "to current service" << service->uuid;
    }

    // add characteristics
    const QList<QLowEnergyCharacteristicData> serviceCharsData = serviceData.characteristics();
    for (const auto &charData: serviceCharsData) {
        // TODO set all characteristic properties
        int permissions = 0;

        //TODO add chars and their descriptors
        QAndroidJniObject javaChar = QAndroidJniObject("android/bluetooth/BluetoothGattCharacteristic",
                                                       "(Ljava/util/UUID;II)V",
                                                       javaUuidfromQtUuid(charData.uuid()).object(),
                                                       int(charData.properties()), permissions);

        QAndroidJniEnvironment env;
        jbyteArray jb = env->NewByteArray(charData.value().size());
        env->SetByteArrayRegion(jb, 0, charData.value().size(), (jbyte*)charData.value().data());
        jboolean success = javaChar.callMethod<jboolean>("setValue", "([B)Z", jb);
        if (!success)
            qWarning() << "Cannot setup initial characteristic value for " << charData.uuid();

        env->DeleteLocalRef(jb);

        // TODO set all descriptors

        success = service->androidService.callMethod<jboolean>(
                            "addCharacteristic",
                            "(Landroid/bluetooth/BluetoothGattCharacteristic;)Z", javaChar.object());
        if (!success)
            qWarning() << "Cannot add characteristic" << charData.uuid() << "to service"
                       << service->uuid;
    }

    hub->javaObject().callMethod<void>("addService",
                                       "(Landroid/bluetooth/BluetoothGattService;)V",
                                       service->androidService.object());
}

QT_END_NAMESPACE
