/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
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
#include "bluetoothlinkmanagerdevicediscoverer.h"
#include "qbluetoothaddress.h"
#include "qbluetoothdeviceinfo.h"
#include "qbluetoothuuid.h"

#include <qstring.h>
#ifdef EIR_SUPPORTED
#include <arpa/inet.h>
#include <netinet/in.h>
#endif
#include "utils_symbian_p.h"
#include <qdebug.h>

/*! \internal
    \class BluetoothLinkManagerDeviceDiscoverer
    \brief The BluetoothLinkManagerDeviceDiscoverer class searches other bluetooth devices.

    \ingroup connectivity-bluetooth
    \inmodule QtBluetooth
    \internal

    BluetoothLinkManagerDeviceDiscoverer is an Symbian ActiveObject derived class that discovers
    other Bluetooth devices using "BTLinkManager" protocol.

    When Bluetoothdevices are found signal deviceDiscovered(QBluetoothDeviceInfo &) is emitted. When
    all devices are found (or none) signal deviceDiscoveryComplete() is emitted.

    In case of an error signal linkManagerError(int error) is emitted. In the case of errorSignal
    one should consider deleting this class and creating it again.
*/

_LIT(KBTLinkManagerTxt,"BTLinkManager");

BluetoothLinkManagerDeviceDiscoverer::BluetoothLinkManagerDeviceDiscoverer(RSocketServ &socketServer,QObject *parent)
    : QObject(parent)
    , CActive(CActive::EPriorityStandard)
    , m_socketServer(socketServer)
    , m_pendingCancel(false), m_pendingStart(false), m_discoveryType (0)
{
    TInt result;

    /* find Bluetooth protocol */
    TProtocolDesc protocol;
    result = m_socketServer.FindProtocol(KBTLinkManagerTxt(),protocol);
    if (result == KErrNone) {
        /* Create and initialise an RHostResolver */
        result = m_hostResolver.Open(m_socketServer, protocol.iAddrFamily, protocol.iProtocol);
    }
    // check possible error
    if (result != KErrNone) {
        setError(result);
    }

    //add this active object to scheduler
    CActiveScheduler::Add(this);
}

BluetoothLinkManagerDeviceDiscoverer::~BluetoothLinkManagerDeviceDiscoverer()
{
    // cancel active object
    Cancel();
    m_hostResolver.Close();
}
/*!
    Starts up device discovery. When devices are discovered signal deviceDiscovered is emitted.
    After signal deviceDiscoveryComplete() is emitted new discovery request can be made.
*/
void BluetoothLinkManagerDeviceDiscoverer::startDiscovery(const uint discoveryType)
{
    m_discoveryType = discoveryType;
    
    if(m_pendingCancel == true) {
        m_pendingStart = true;
        m_pendingCancel = false;  
        return;
    }
    if (!IsActive()) {
        m_addr.SetIAC( discoveryType );
#ifdef EIR_SUPPORTED
        m_addr.SetAction(KHostResInquiry | KHostResName | KHostResIgnoreCache | KHostResEir);
#else
        m_addr.SetAction(KHostResInquiry | KHostResName | KHostResIgnoreCache);
#endif
        m_hostResolver.GetByAddress(m_addr, m_entry, iStatus);
        SetActive();
    }
}

void BluetoothLinkManagerDeviceDiscoverer::stopDiscovery()
{
    m_pendingStart = false; 
    if (IsActive() && !m_pendingCancel) {
        m_pendingCancel = true;
        m_hostResolver.Cancel();
    }
}

/*!
  Informs that our request has been prosessed and the data is available to be used.
*/
void BluetoothLinkManagerDeviceDiscoverer::RunL()
{
    qDebug() << __PRETTY_FUNCTION__ << iStatus.Int();
    switch (iStatus.Int()) {
    case KErrNone:  // found device
        if (m_pendingCancel && !m_pendingStart) {
            m_pendingCancel = false;
            emit canceled();
        } else {
            m_pendingCancel = false;
            m_pendingStart = false;
            // get next (possible) discovered device
            m_hostResolver.Next(m_entry, iStatus);
            SetActive();
            emit deviceDiscovered(currentDeviceDataToQBluetoothDeviceInfo());
        }
        break;
    case KErrHostResNoMoreResults:  // done with search
        if (m_pendingCancel && !m_pendingStart){  // search was canceled prior to finishing
            m_pendingCancel = false;
            m_pendingStart = false;
            emit canceled();
        }
        else if (m_pendingStart){  // search was restarted just prior to finishing
            m_pendingStart = false;
            m_pendingCancel = false;
            startDiscovery(m_discoveryType);
        } else {  // search completed normally
            m_pendingStart = false;
            m_pendingCancel = false;
            emit deviceDiscoveryComplete();
        }
        break;
    case KErrCancel:  // user canceled search
        if (m_pendingStart){  // user activated search after cancel
            m_pendingStart = false;
            m_pendingCancel = false;
            startDiscovery(m_discoveryType);
        } else {  // standard user cancel case
            m_pendingCancel = false;
            emit canceled();
        }
        break;
    default:
        m_pendingStart = false;
        m_pendingCancel = false;
        setError(iStatus.Int());
    }
}
/*!
  Cancel current request.
*/
void BluetoothLinkManagerDeviceDiscoverer::DoCancel()
{
    m_hostResolver.Cancel();
}

TInt BluetoothLinkManagerDeviceDiscoverer::RunError(TInt aError)
{
    setError(aError);
    return KErrNone;
}

bool BluetoothLinkManagerDeviceDiscoverer::isReallyActive() const
{
    if(m_pendingStart)
        return true;
    if(m_pendingCancel)
        return false;
    return IsActive();
}

/*!
    Transforms Symbian device name, address and service classes to QBluetootDeviceInfo.
*/
QBluetoothDeviceInfo BluetoothLinkManagerDeviceDiscoverer::currentDeviceDataToQBluetoothDeviceInfo() const
{
    // extract device information from results and map them to QBluetoothDeviceInfo
#ifdef EIR_SUPPORTED
    TBluetoothNameRecordWrapper eir(m_entry());
    TInt bufferlength = 0;
    TInt err = KErrNone;
    QString deviceName;
    bufferlength = eir.GetDeviceNameLength();

    if (bufferlength > 0) {
        TBool nameComplete;
        HBufC *deviceNameBuffer = 0;
        TRAP(err,deviceNameBuffer = HBufC::NewL(bufferlength));
        if(err)
            deviceName = QString();
        else
            {
            TPtr ptr = deviceNameBuffer->Des();
            err = eir.GetDeviceName(ptr,nameComplete);
            if (err == KErrNone /*&& nameComplete*/)
                {
                if(!nameComplete)
                    qWarning() << "device name incomplete";
                // isn't it better to get an incomplete name than getting nothing?
                deviceName = QString::fromUtf16(ptr.Ptr(), ptr.Length()).toUpper();
                }
            else
                deviceName = QString();
            delete deviceNameBuffer;
            }
    }

    QList<QBluetoothUuid> serviceUidList;
    RExtendedInquiryResponseUUIDContainer uuidContainer;
    QBluetoothDeviceInfo::DataCompleteness completenes = QBluetoothDeviceInfo::DataUnavailable;

    if (eir.GetServiceClassUuids(uuidContainer) == KErrNone) {
        TInt uuidCount = uuidContainer.UUIDs().Count();
        if (uuidCount > 0) {
            for (int i = 0; i < uuidCount; ++i) {
                TPtrC8 shortFormUUid(uuidContainer.UUIDs()[i].ShortestForm());
                if (shortFormUUid.Size() == 2) {
                    QBluetoothUuid uuid(ntohs(*reinterpret_cast<const quint16 *>(shortFormUUid.Ptr())));
                    if (uuidContainer.GetCompleteness(RExtendedInquiryResponseUUIDContainer::EUUID16))
                        completenes = QBluetoothDeviceInfo::DataComplete;
                    else
                        completenes = QBluetoothDeviceInfo::DataIncomplete;
                    serviceUidList.append(uuid);
                }else if (shortFormUUid.Size() == 4) {
                    QBluetoothUuid uuid(ntohl(*reinterpret_cast<const quint32 *>(shortFormUUid.Ptr())));
                    if (uuidContainer.GetCompleteness(RExtendedInquiryResponseUUIDContainer::EUUID32))
                        completenes = QBluetoothDeviceInfo::DataComplete;
                    else
                        completenes = QBluetoothDeviceInfo::DataIncomplete;
                    serviceUidList.append(uuid);
                }else if (shortFormUUid.Size() == 16) {
                    QBluetoothUuid uuid(*reinterpret_cast<const quint128 *>(shortFormUUid.Ptr()));
                    if (uuidContainer.GetCompleteness(RExtendedInquiryResponseUUIDContainer::EUUID128))
                        completenes = QBluetoothDeviceInfo::DataComplete;
                    else
                        completenes = QBluetoothDeviceInfo::DataIncomplete;
                    serviceUidList.append(uuid);
                }
            }
        }
    }
    uuidContainer.Close();

    bufferlength = 0;
    QByteArray manufacturerData;
    bufferlength = eir.GetVendorSpecificDataLength();

    if (bufferlength > 0) {
        HBufC8 *msd = 0;
        TRAP(err,msd = HBufC8::NewL(bufferlength));
        if(err)
            manufacturerData = QByteArray();
        else
            {
            TPtr8 temp = msd->Des();
            if (eir.GetVendorSpecificData(temp))
                manufacturerData = s60Desc8ToQByteArray(temp);
            else
                manufacturerData = QByteArray();
            delete msd;
            }
    }

    // Get transmission power level
    TInt8 transmissionPowerLevel = 0;
    eir.GetTxPowerLevel(transmissionPowerLevel);

    // unique address of the device
    const TBTDevAddr symbianDeviceAddress = static_cast<TBTSockAddr> (m_entry().iAddr).BTAddr();
    QBluetoothAddress bluetoothAddress = qTBTDevAddrToQBluetoothAddress(symbianDeviceAddress);

    // format symbian major/minor numbers
    quint32 deviceClass = qTPackSymbianDeviceClass(m_addr);

    QBluetoothDeviceInfo deviceInfo(bluetoothAddress, deviceName, deviceClass);

    deviceInfo.setRssi(transmissionPowerLevel);
    deviceInfo.setServiceUuids(serviceUidList, completenes);
    deviceInfo.setManufacturerSpecificData(manufacturerData);
#else
    // device name
    THostName symbianDeviceName = m_entry().iName;
    QString deviceName = QString::fromUtf16(symbianDeviceName.Ptr(), symbianDeviceName.Length()).toUpper();

    // unique address of the device
    const TBTDevAddr symbianDeviceAddress = static_cast<TBTSockAddr> (m_entry().iAddr).BTAddr();
    QBluetoothAddress bluetoothAddress = qTBTDevAddrToQBluetoothAddress(symbianDeviceAddress);

    // format symbian major/minor numbers
    quint32 deviceClass = qTPackSymbianDeviceClass(m_addr);

    QBluetoothDeviceInfo deviceInfo(bluetoothAddress, deviceName, deviceClass);

    if (m_addr.Rssi())
        deviceInfo.setRssi(m_addr.Rssi());
#endif
    if (!deviceInfo.rssi())
        deviceInfo.setRssi(1);

    deviceInfo.setCached(false);  //TODO cache support missing from devicediscovery API
    //qDebug()<< "Discovered device: name="<< deviceName <<", address=" << bluetoothAddress.toString() <<", class=" << deviceClass;
    return deviceInfo;
}

void BluetoothLinkManagerDeviceDiscoverer::setError(int errorCode)
{
    qDebug() << __PRETTY_FUNCTION__ << "errorCode=" << errorCode;
    QString errorString;
    switch (errorCode) {
        case KLinkManagerErrBase:
            errorString.append("Link manager base error value or Insufficient baseband resources error value");
            emit linkManagerError(QBluetoothDeviceDiscoveryAgent::UnknownError, errorString);
            break;
        case KErrProxyWriteNotAvailable:
            errorString.append("Proxy write not available error value");
            emit linkManagerError(QBluetoothDeviceDiscoveryAgent::UnknownError, errorString);
            break;
        case KErrReflexiveBluetoothLink:
            errorString.append("Reflexive BT link error value");
            emit linkManagerError(QBluetoothDeviceDiscoveryAgent::UnknownError, errorString);
            break;
        case KErrPendingPhysicalLink:
            errorString.append("Physical link connection already pending when trying to connect the physical link");
            emit linkManagerError(QBluetoothDeviceDiscoveryAgent::UnknownError, errorString);
            break;
        case KErrNotReady:
            errorString.append("KErrNotReady");
            emit linkManagerError(QBluetoothDeviceDiscoveryAgent::UnknownError, errorString);
        case KErrCancel:
            errorString.append("KErrCancel");
            qDebug() << "emitting canceled";
            emit canceled();
            break;
        case KErrNone:
            // do nothing
            break;
        default:
            errorString = QString("Symbian errorCode = %1").arg(errorCode);
            emit linkManagerError(QBluetoothDeviceDiscoveryAgent::UnknownError, errorString);
            break;
    }
}
#include "moc_bluetoothlinkmanagerdevicediscoverer.cpp"
