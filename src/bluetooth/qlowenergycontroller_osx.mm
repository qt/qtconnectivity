/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Copyright (C) 2013 Javier S. Pedro <maemo@javispedro.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
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
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "osx/osxbtutility_p.h"

#include "qlowenergyserviceprivate_p.h"
#include "qlowenergycontroller_osx_p.h"
#include "qbluetoothlocaldevice.h"
#include "qbluetoothdeviceinfo.h"
#include "qlowenergycontroller.h"
#include "qbluetoothuuid.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>

#define OSX_D_PTR QLowEnergyControllerPrivateOSX *osx_d_ptr = static_cast<QLowEnergyControllerPrivateOSX *>(d_ptr)

QT_BEGIN_NAMESPACE

QLowEnergyControllerPrivateOSX::QLowEnergyControllerPrivateOSX(QLowEnergyController *q)
    : q_ptr(q),
      isConnecting(false),
      lastError(QLowEnergyController::NoError),
      controllerState(QLowEnergyController::UnconnectedState),
      addressType(QLowEnergyController::PublicAddress)
{
    // This is the "wrong" constructor - no valid device UUID to connect later.
    Q_ASSERT_X(q, "QLowEnergyControllerPrivate", "invalid q_ptr (null)");
    // We still create a manager, to simplify error handling later.
    centralManager.reset([[ObjCCentralManager alloc] initWithDelegate:this]);
    if (!centralManager) {
        qCWarning(QT_BT_OSX) << "QBluetoothLowEnergyControllerPrivateOSX::"
                                "QBluetoothLowEnergyControllerPrivateOSX(), "
                                "failed to initialize central manager";
    }

}

QLowEnergyControllerPrivateOSX::QLowEnergyControllerPrivateOSX(QLowEnergyController *q,
                                                               const QBluetoothDeviceInfo &deviceInfo)
    : q_ptr(q),
      deviceUuid(deviceInfo.deviceUuid()),
      isConnecting(false),
      lastError(QLowEnergyController::NoError),
      controllerState(QLowEnergyController::UnconnectedState),
      addressType(QLowEnergyController::PublicAddress)
{
    Q_ASSERT_X(q, "QLowEnergyControllerPrivateOSX", "invalid q_ptr (null)");
    centralManager.reset([[ObjCCentralManager alloc] initWithDelegate:this]);
    if (!centralManager) {
        qCWarning(QT_BT_OSX) << "QBluetoothLowEnergyControllerPrivateOSX::"
                                "QBluetoothLowEnergyControllerPrivateOSX(), "
                                "failed to initialize central manager";
    }
}

QLowEnergyControllerPrivateOSX::~QLowEnergyControllerPrivateOSX()
{
}

bool QLowEnergyControllerPrivateOSX::isValid() const
{
    // isValid means only "was able to allocate all resources",
    // nothing more.
    return centralManager;
}

void QLowEnergyControllerPrivateOSX::LEnotSupported()
{
    // Report as an error. But this should not be possible
    // actually: before connecting to any device, we have
    // to discover it, if it was discovered ... LE _must_
    // be supported.
}

void QLowEnergyControllerPrivateOSX::connectSuccess()
{
    Q_ASSERT_X(controllerState == QLowEnergyController::ConnectingState,
               "connectSuccess", "invalid state");

    controllerState = QLowEnergyController::ConnectedState;

    if (!isConnecting) {
        emit q_ptr->stateChanged(QLowEnergyController::ConnectedState);
        emit q_ptr->connected();
    }
}

void QLowEnergyControllerPrivateOSX::serviceDiscoveryFinished(LEServices services)
{
    Q_ASSERT_X(controllerState == QLowEnergyController::DiscoveringState,
               "serviceDiscoveryFinished", "invalid state");

    QT_BT_MAC_AUTORELEASEPOOL;

    for (CBService *service in services.data()) {
        if (CBUUID *const uuid = service.UUID) {
            const QBluetoothUuid qtUuid(OSXBluetooth::qt_uuid(uuid));
            if (discoveredServices.find(qtUuid) != discoveredServices.end()) {
                qCDebug(QT_BT_OSX) << "QBluetoothLowEnergyControllerPrivateOSX::serviceDiscoveryFinished(), "
                                      "a duplicate service UUID found: " << qtUuid;
                continue;
            }

            QSharedPointer<QLowEnergyServicePrivate> newService(new QLowEnergyServicePrivate);
            newService->uuid = qtUuid;
#if QT_MAC_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_9, __IPHONE_6_0)
            if (!service.isPrimary) {
                newService->type &= ~QLowEnergyService::PrimaryService;
                newService->type |= QLowEnergyService::IncludedService;
            }
#endif
            newService->setController(this);
            discoveredServices.insert(qtUuid, newService);
            emit q_ptr->serviceDiscovered(qtUuid);
        }
    }

    controllerState = QLowEnergyController::DiscoveredState;
    emit q_ptr->stateChanged(QLowEnergyController::DiscoveredState);
    emit q_ptr->discoveryFinished();
}

void QLowEnergyControllerPrivateOSX::includedServicesDiscoveryFinished(const QBluetoothUuid &serviceUuid,
                                                                       LEServices services)
{
    Q_UNUSED(serviceUuid)
    Q_UNUSED(services)
}

void QLowEnergyControllerPrivateOSX::characteristicsDiscoveryFinished(const QBluetoothUuid &serviceUuid,
                                                                      LECharacteristics characteristics)
{
    Q_UNUSED(serviceUuid)
    Q_UNUSED(characteristics)
}


void QLowEnergyControllerPrivateOSX::disconnected()
{
    controllerState = QLowEnergyController::UnconnectedState;

    if (!isConnecting) {
        emit q_ptr->stateChanged(QLowEnergyController::UnconnectedState);
        emit q_ptr->disconnected();
    }
}

void QLowEnergyControllerPrivateOSX::error(QLowEnergyController::Error errorCode)
{
    // Errors reported during connect and general errors.
    lastError = errorCode;

    // We're still in connectToDevice,
    // some error was reported synchronously.
    // Return, the error will be correctly set later
    // by connectToDevice.
    if (isConnecting)
        return;

    switch (lastError) {
    case QLowEnergyController::UnknownRemoteDeviceError:
        errorString = QLowEnergyController::tr("Remote device cannot be found");
        break;
    case QLowEnergyController::InvalidBluetoothAdapterError:
        errorString = QLowEnergyController::tr("Cannot find local adapter");
        break;
    case QLowEnergyController::NetworkError:
        errorString = QLowEnergyController::tr("Error occurred during connection I/O");
        break;
    case QLowEnergyController::UnknownError:
    default:
        errorString = QLowEnergyController::tr("Unknown Error");
        break;
    }

    emit q_ptr->error(lastError);
}

void QLowEnergyControllerPrivateOSX::error(const QBluetoothUuid &serviceUuid,
                                           QLowEnergyController::Error errorCode)
{
    // Errors reported while discovering service details etc.
    Q_UNUSED(serviceUuid)
    Q_UNUSED(errorCode)
}

void QLowEnergyControllerPrivateOSX::connectToDevice()
{
    Q_ASSERT_X(isValid(), "connectToDevice", "invalid private controller");
    Q_ASSERT_X(controllerState == QLowEnergyController::UnconnectedState,
               "connectToDevice", "invalid state");
    Q_ASSERT_X(!deviceUuid.isNull(), "connectToDevice",
               "invalid private controller (no device uuid)");
    Q_ASSERT_X(!isConnecting, "connectToDevice",
               "recursive connectToDevice call");

    lastError = QLowEnergyController::NoError;
    errorString.clear();

    isConnecting = true;// Do not emit signals if some callback is executed synchronously.
    controllerState = QLowEnergyController::ConnectingState;
    const QLowEnergyController::Error status = [centralManager connectToDevice:deviceUuid];
    isConnecting = false;

    if (status == QLowEnergyController::NoError && lastError == QLowEnergyController::NoError) {
        emit q_ptr->stateChanged(controllerState);
        if (controllerState == QLowEnergyController::ConnectedState) {
            // If a peripheral is connected already from the Core Bluetooth's
            // POV:
            emit q_ptr->connected();
        } else if (controllerState == QLowEnergyController::UnconnectedState) {
            // Ooops, tried to connect, got peripheral disconnect instead -
            // this happens with Core Bluetooth.
            emit q_ptr->disconnected();
        }
    } else if (status != QLowEnergyController::NoError) {
        error(status);
    } else {
        // Re-set the error/description and emit.
        error(lastError);
    }
}

void QLowEnergyControllerPrivateOSX::discoverServices()
{
    Q_ASSERT_X(isValid(), "discoverServices", "invalid private controller");
    Q_ASSERT_X(controllerState != QLowEnergyController::UnconnectedState,
               "discoverServices", "not connected to peripheral");

    controllerState = QLowEnergyController::DiscoveringState;
    emit q_ptr->stateChanged(QLowEnergyController::DiscoveringState);
    [centralManager discoverServices];
}

void QLowEnergyControllerPrivateOSX::discoverServiceDetails(const QBluetoothUuid &serviceUuid)
{
    Q_UNUSED(serviceUuid);
}

QLowEnergyController::QLowEnergyController(const QBluetoothAddress &remoteAddress,
                                           QObject *parent)
    : QObject(parent),
      d_ptr(new QLowEnergyControllerPrivateOSX(this))
{
    Q_UNUSED(remoteAddress)

    qCWarning(QT_BT_OSX) << "QLowEnergyController::QLowEnergyController(), "
                            "construction with remote address is not supported!";
}

QLowEnergyController::QLowEnergyController(const QBluetoothDeviceInfo &remoteDevice,
                                           QObject *parent)
    : QObject(parent),
      d_ptr(new QLowEnergyControllerPrivateOSX(this, remoteDevice))
{
    // That's the only "real" ctor - with Core Bluetooth we need a _valid_ deviceUuid
    // from 'remoteDevice'.
}

QLowEnergyController::QLowEnergyController(const QBluetoothAddress &remoteAddress,
                                           const QBluetoothAddress &localAddress,
                                           QObject *parent)
    : QObject(parent),
      d_ptr(new QLowEnergyControllerPrivateOSX(this))
{
    OSX_D_PTR;

    osx_d_ptr->remoteAddress = remoteAddress;
    osx_d_ptr->localAddress = localAddress;

    qCWarning(QT_BT_OSX) << "QLowEnergyController::QLowEnergyController(), "
                            "construction with remote/local addresses is not supported!";
}

QLowEnergyController::~QLowEnergyController()
{
    // Deleting a peripheral will also disconnect.
    delete d_ptr;
}

QBluetoothAddress QLowEnergyController::localAddress() const
{
    OSX_D_PTR;

    return osx_d_ptr->localAddress;
}

QBluetoothAddress QLowEnergyController::remoteAddress() const
{
    OSX_D_PTR;

    return osx_d_ptr->remoteAddress;
}

QLowEnergyController::ControllerState QLowEnergyController::state() const
{
    OSX_D_PTR;

    return osx_d_ptr->controllerState;
}

QLowEnergyController::RemoteAddressType QLowEnergyController::remoteAddressType() const
{
    OSX_D_PTR;

    return osx_d_ptr->addressType;
}

void QLowEnergyController::setRemoteAddressType(RemoteAddressType type)
{
    Q_UNUSED(type)

    OSX_D_PTR;

    osx_d_ptr->addressType = type;
}

void QLowEnergyController::connectToDevice()
{
    OSX_D_PTR;

    // A memory allocation problem.
    if (!osx_d_ptr->isValid())
        return osx_d_ptr->error(UnknownError);

    // No QBluetoothDeviceInfo provided during construction.
    if (osx_d_ptr->deviceUuid.isNull())
        return osx_d_ptr->error(UnknownRemoteDeviceError);

    if (osx_d_ptr->controllerState != UnconnectedState)
        return;

    osx_d_ptr->connectToDevice();
}

void QLowEnergyController::disconnectFromDevice()
{
    if (state() == UnconnectedState || state() == ClosingState)
        return;

    OSX_D_PTR;

    if (osx_d_ptr->isValid()) {
        const ControllerState oldState = osx_d_ptr->controllerState;

        osx_d_ptr->controllerState = ClosingState;
        emit stateChanged(ClosingState);
        [osx_d_ptr->centralManager disconnectFromDevice];

        if (oldState == ConnectingState) {
            // With a pending connect attempt there is no
            // guarantee we'll ever have didDisconnect callback,
            // set the state here and now to make sure we still
            // can connect.
            osx_d_ptr->controllerState = UnconnectedState;
            emit stateChanged(UnconnectedState);
        }
    }
}

void QLowEnergyController::discoverServices()
{
    if (state() != ConnectedState)
        return;

    OSX_D_PTR;

    osx_d_ptr->discoverServices();
}

QList<QBluetoothUuid> QLowEnergyController::services() const
{
    OSX_D_PTR;

    return osx_d_ptr->discoveredServices.keys();
}

QLowEnergyService *QLowEnergyController::createServiceObject(const QBluetoothUuid &serviceUuid,
                                                             QObject *parent)
{
    OSX_D_PTR;

    if (!osx_d_ptr->discoveredServices.contains(serviceUuid))
        return Q_NULLPTR;

    return new QLowEnergyService(osx_d_ptr->discoveredServices.value(serviceUuid), parent);
}

QLowEnergyController::Error QLowEnergyController::error() const
{
    OSX_D_PTR;

    return osx_d_ptr->lastError;
}

QString QLowEnergyController::errorString() const
{
    OSX_D_PTR;

    return osx_d_ptr->errorString;
}

QT_END_NAMESPACE
