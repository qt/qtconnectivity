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
      lastError(QLowEnergyController::NoError),
      controllerState(QLowEnergyController::UnconnectedState),
      addressType(QLowEnergyController::PublicAddress)
{
    Q_ASSERT_X(q, "QLowEnergyControllerPrivate", "invalid q_ptr (null)");
    // This is the "wrong" constructor - no valid device UUID to connect later.
}

QLowEnergyControllerPrivateOSX::QLowEnergyControllerPrivateOSX(QLowEnergyController *q,
                                                               const QBluetoothDeviceInfo &deviceInfo)
    : q_ptr(q),
      deviceUuid(deviceInfo.deviceUuid()),
      lastError(QLowEnergyController::NoError),
      controllerState(QLowEnergyController::UnconnectedState),
      addressType(QLowEnergyController::PublicAddress)
{
    Q_ASSERT_X(q, "QLowEnergyControllerPrivateOSX", "invalid q_ptr (null)");
}

QLowEnergyControllerPrivateOSX::~QLowEnergyControllerPrivateOSX()
{
}

bool QLowEnergyControllerPrivateOSX::isValid() const
{
    return false;
}

void QLowEnergyControllerPrivateOSX::connectToDevice()
{
}

void QLowEnergyControllerPrivateOSX::discoverServices()
{
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
}

QLowEnergyController::QLowEnergyController(const QBluetoothDeviceInfo &remoteDevice,
                                           QObject *parent)
    : QObject(parent),
      d_ptr(new QLowEnergyControllerPrivateOSX(this, remoteDevice))
{
    Q_UNUSED(remoteDevice)
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
                            "construction with remote address is not supported";
}

QLowEnergyController::~QLowEnergyController()
{
    // TODO: disconnect, but this can be quite problematic - it's async. operation!
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
}

void QLowEnergyController::disconnectFromDevice()
{
}

void QLowEnergyController::discoverServices()
{
}

QList<QBluetoothUuid> QLowEnergyController::services() const
{
    return QList<QBluetoothUuid>();
}

QLowEnergyService *QLowEnergyController::createServiceObject(const QBluetoothUuid &serviceUuid,
                                                             QObject *parent)
{
    Q_UNUSED(serviceUuid)
    Q_UNUSED(parent)
    return Q_NULLPTR;
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
