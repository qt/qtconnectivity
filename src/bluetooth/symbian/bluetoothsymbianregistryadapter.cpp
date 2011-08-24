/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

// INCLUDE FILES
#include "bluetoothsymbianregistryadapter.h"
#include "qbluetoothaddress.h"
#include "qbluetoothdeviceinfo.h"
#include <qstring.h>
#include <btdevice.h>
#include "utils_symbian_p.h"

/*! \internal
    \class BluetoothSymbianRegistryAdapter
    \brief The BluetoothSymbianRegistryAdapter class is an adapter for handling Symbian Bluetooth
    Registry.

    The BluetoothSymbianRegistryAdapter is constructed to use for a one QBluetoothAddress.
    It uses following Symbian class CBTEngDevMan for native operations.

    \ingroup connectivity-bluetooth
    \inmodule QtConnectivity
    \internal

    @internalComponent

*/

BluetoothSymbianRegistryAdapter::BluetoothSymbianRegistryAdapter(const QBluetoothAddress &address, QObject *parent)
    : QObject(parent)
    , m_bluetoothDeviceManager(0)
    , m_address(address)
    , m_operation(NoOp)
{
    TRAP(m_errorCode, m_bluetoothDeviceManager = CBTEngDevMan::NewL(this))
    if (m_errorCode != KErrNone) 
        emit registryHandlingError(m_errorCode);
}
BluetoothSymbianRegistryAdapter::~BluetoothSymbianRegistryAdapter()
{
    delete m_bluetoothDeviceManager;
}

int BluetoothSymbianRegistryAdapter::errorCode() const
{
    return m_errorCode;
}

QString BluetoothSymbianRegistryAdapter::pairingErrorString() const
{
    return m_pairingErrorString;
}

QBluetoothLocalDevice::Pairing BluetoothSymbianRegistryAdapter::remoteDevicePairingStatus()
{
    QBluetoothLocalDevice::Pairing returnValue = QBluetoothLocalDevice::Unpaired;
    // if the address provided is not valid
    if (m_address.isNull()) {
        emit registryHandlingError((int)KErrArgument);
        return returnValue;
    }
    int errorCode = 0;
    // use Symbian registry to get a current pairing status of a device
    // do not pass observer for this method -> use sync version of GetDevices()
    CBTEngDevMan *btDeviceManager = NULL;
    TRAPD(deviceManagerCreationErr, btDeviceManager = CBTEngDevMan::NewL(NULL));
    if (deviceManagerCreationErr != KErrNone) {
        emit registryHandlingError((int)deviceManagerCreationErr);
        return returnValue;
    }

    // Spesify search criteria
    TBTRegistrySearch lSearchCriteria;
    TBTDevAddr btAddress(m_address.toUInt64());
    lSearchCriteria.FindAddress(btAddress);

    CBTDeviceArray *listOfDevices = NULL;
    TRAPD(arrayCreationErr,listOfDevices = createDeviceArrayL());
    if (arrayCreationErr !=KErrNone) {
        emit registryHandlingError((int)arrayCreationErr);
        // cleanup manager before returning
        delete btDeviceManager;
        return returnValue;
    }
    errorCode = btDeviceManager->GetDevices(lSearchCriteria,listOfDevices);
    if (errorCode != KErrNone) {
        emit registryHandlingError(errorCode);
        return returnValue;
    }
    if ( listOfDevices->Count() == 1) {
        CBTDevice *device= listOfDevices->At(0);
        bool isValidDevicePaired = device->IsValidPaired();
        bool isDevicePaired = device->IsPaired();
        //TODO Check whether check is correct?
        if (isValidDevicePaired || isDevicePaired) {
            if (device->GlobalSecurity().NoAuthorise())
                returnValue = QBluetoothLocalDevice::AuthorizedPaired;
            else
                returnValue = QBluetoothLocalDevice::Paired;
        }
        else if (!isValidDevicePaired && !isDevicePaired) {
            returnValue = QBluetoothLocalDevice::Unpaired;
        }

    } else {
        returnValue = QBluetoothLocalDevice::Unpaired;
    }
    //cleanup
    listOfDevices->ResetAndDestroy();
    delete listOfDevices;
    delete btDeviceManager;
    return returnValue;
}
CBTDeviceArray* BluetoothSymbianRegistryAdapter::createDeviceArrayL() const
{
    return q_check_ptr(new CBTDeviceArray(10));
}

void BluetoothSymbianRegistryAdapter::removePairing()
{
    // setup current values
    int errorCode = 0;
    m_operation = RemovePairing;
    // Spesify search criteria
    TBTRegistrySearch searchCriteria;
    TBTDevAddr btAddress(m_address.toUInt64());
    searchCriteria.FindAddress(btAddress);
    errorCode = m_bluetoothDeviceManager->DeleteDevices(searchCriteria);
    // if there is error then manually call callback for error routines
    if (errorCode != KErrNone)
        HandleDevManComplete(errorCode);

}

QBluetoothLocalDevice::Pairing BluetoothSymbianRegistryAdapter::pairingStatus()
{
    m_operation = PairingStatus;  // only needed for uniformity
    QBluetoothLocalDevice::Pairing pairStatus = remoteDevicePairingStatus();
    m_operation = NoOp;
    
    return pairStatus;
}

void BluetoothSymbianRegistryAdapter::HandleDevManComplete( TInt aErr )
{
    m_errorCode = aErr;
    if (aErr != KErrNone) {
        emit registryHandlingError(aErr);
    }
    if (m_operation == RemovePairing) {
        emit pairingStatusChanged(m_address, QBluetoothLocalDevice::Unpaired);
        m_operation = NoOp;
    }

}

void BluetoothSymbianRegistryAdapter::HandleGetDevicesComplete( TInt aErr,CBTDeviceArray* aDeviceArray )
{
    Q_UNUSED(aErr);
    Q_UNUSED(aDeviceArray);
}

#include "moc_bluetoothsymbianregistryadapter.cpp"
