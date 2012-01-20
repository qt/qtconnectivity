/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include "qbluetoothlocaldevice_p.h"

#include "qbluetoothlocaldevice.h"
#include <QtCore/QString>
#include "symbian/utils_symbian_p.h"
#include <bttypes.h>
#include <bt_subscribe.h>
#ifdef USING_BTENGCONNMAN
#include "bluetoothsymbianpairingadapter.h"
#endif //USING_BTENGCONNMAN
#ifdef USING_BTENGDEVMAN
#include "bluetoothsymbianregistryadapter.h"
#endif //USING_BTENGDEVMAN

QTBLUETOOTH_BEGIN_NAMESPACE

QBluetoothLocalDevicePrivate::QBluetoothLocalDevicePrivate()
    : m_settings(NULL)
{
    TRAPD(err, m_settings = CBTEngSettings::NewL(this));
    if (err != KErrNone) {
        Q_Q(QBluetoothLocalDevice);
        emit q->error(QBluetoothLocalDevice::UnknownError);
        m_settings = NULL;
    }
}

QBluetoothLocalDevicePrivate::~QBluetoothLocalDevicePrivate()
{
    delete m_settings;
}

QString QBluetoothLocalDevicePrivate::name()
{
    CBTEngSettings *settings = NULL;
    TRAPD(err, settings = CBTEngSettings::NewL());
    if (err != KErrNone)
        return QString();
    HBufC *localName = NULL;
    TRAPD(error, localName = HBufC::NewL(256));
    if (error != KErrNone) {
        delete settings;
        return QString();
    }
    TPtr localPtr = localName->Des();
    TInt errorCode = settings->GetLocalName(localPtr);
    QString name;
    if (errorCode == KErrNone)
        name = s60DescToQString(localPtr);

    delete localName;
    delete settings;
    return name;
}

QBluetoothAddress QBluetoothLocalDevicePrivate::address()
{
    TBuf<20> bluetoothAddress;
    TPckgBuf<TBTDevAddr> addressPackage;

    TInt error = RProperty::Get(KUidSystemCategory, KPropertyKeyBluetoothGetLocalDeviceAddress, addressPackage);

    if (error != KErrNone)
        return QBluetoothAddress();

    addressPackage().GetReadable(bluetoothAddress, KNullDesC, _L(":"), KNullDesC);

    return QBluetoothAddress(s60DescToQString(bluetoothAddress));
}

void QBluetoothLocalDevicePrivate::powerOn()
{
    if (!m_settings)
        return;
    TBTPowerStateValue powerState;
    TInt error = m_settings->GetPowerState(powerState);
    if (error == KErrNone && powerState == EBTPowerOff)
        m_settings->SetPowerState(EBTPowerOn);
    else if (error != KErrNone) {
        Q_Q(QBluetoothLocalDevice);
        emit q->error(QBluetoothLocalDevice::UnknownError);
    }
}
void QBluetoothLocalDevicePrivate::powerOff()
{
    if (!m_settings)
        return;
    TBTPowerStateValue powerState;
    TInt error = m_settings->GetPowerState(powerState);
    if (error == KErrNone && powerState == EBTPowerOn)
        m_settings->SetPowerState(EBTPowerOff);
    else if (error != KErrNone) {
        Q_Q(QBluetoothLocalDevice);
        emit q->error(QBluetoothLocalDevice::UnknownError);
    }
}
void QBluetoothLocalDevicePrivate::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    if (!m_settings)
        return;

    TInt error = KErrNone;
    switch (mode) {
        case QBluetoothLocalDevice::HostPoweredOff:
            powerOff();
            break;
        case QBluetoothLocalDevice::HostConnectable: {
            TBTPowerStateValue powerState;
            error = m_settings->GetPowerState(powerState);
            if (error == KErrNone) {
                if (powerState == EBTPowerOff) {
                    error = m_settings->SetPowerState(EBTPowerOn);
                }
                TBTVisibilityMode visibilityMode;
                error = m_settings->GetVisibilityMode(visibilityMode);
                if (visibilityMode != EBTVisibilityModeHidden) {
                    error = m_settings->SetVisibilityMode(EBTVisibilityModeHidden);
                }
            }
            break;
        }
        case QBluetoothLocalDevice::HostDiscoverable: {
            TBTPowerStateValue powerState;
            error = m_settings->GetPowerState(powerState);
            if (error == KErrNone) {
                if (powerState == EBTPowerOff) {
                    error = m_settings->SetPowerState(EBTPowerOn);
                }
                TBTVisibilityMode visibilityMode;
                error = m_settings->GetVisibilityMode(visibilityMode);
                if (visibilityMode != EBTVisibilityModeGeneral) {
                    error = m_settings->SetVisibilityMode(EBTVisibilityModeGeneral);
                }
            }
            break;
        }
    }
    if (error != KErrNone) {
        Q_Q(QBluetoothLocalDevice);
        emit q->error(QBluetoothLocalDevice::UnknownError);
    }
}


QBluetoothLocalDevice::HostMode QBluetoothLocalDevicePrivate::hostMode() const
{
    if (!m_settings)
        return QBluetoothLocalDevice::HostPoweredOff;

    TBTVisibilityMode visibilityMode;
    TInt error = m_settings->GetVisibilityMode(visibilityMode);

    if (error != KErrNone)
        return QBluetoothLocalDevice::HostPoweredOff;

    //If we are powered off then we don't are neither connectable or discoverable
    TBTPowerStateValue powerState;
    error = m_settings->GetPowerState(powerState);
    if (error == KErrNone && powerState == EBTPowerOff)
        return QBluetoothLocalDevice::HostPoweredOff;

    switch (visibilityMode) {
        case EBTVisibilityModeHidden:
            return QBluetoothLocalDevice::HostConnectable;
        case EBTVisibilityModeGeneral:
        case EBTVisibilityModeTemporary:
            return QBluetoothLocalDevice::HostDiscoverable;
        default:
            // default value, also includes these new values from Symbian^3 onwards
            // case EBTVisibilityModeNoScans:
            // case EBTVisibilityModeInquiryScanOnly:
            return QBluetoothLocalDevice::HostPoweredOff;
    }
}

void QBluetoothLocalDevicePrivate::PowerStateChanged(TBTPowerStateValue aState)
{
    QBluetoothLocalDevice::HostMode hostMode;
    switch (aState) {
        case EBTPowerOn:
            hostMode = this->hostMode();
            break;
        case EBTPowerOff:
        default:
            hostMode = QBluetoothLocalDevice::HostPoweredOff;
            break;
    }
    Q_Q(QBluetoothLocalDevice);
    emit q->hostModeStateChanged(hostMode);
}

void QBluetoothLocalDevicePrivate::VisibilityModeChanged(TBTVisibilityMode aState)
{
    QBluetoothLocalDevice::HostMode hostMode;
    switch (aState) {
        case EBTVisibilityModeHidden:
            hostMode = QBluetoothLocalDevice::HostConnectable;
            break;
        case EBTVisibilityModeGeneral:
        case EBTVisibilityModeTemporary:
            hostMode = QBluetoothLocalDevice::HostDiscoverable;
            break;
        default:
            // default value, also includes these new values from Symbian^3 onwards
            // case EBTVisibilityModeNoScans:
            // case EBTVisibilityModeInquiryScanOnly:
            hostMode = QBluetoothLocalDevice::HostPoweredOff;
            break;
    }
    Q_Q(QBluetoothLocalDevice);
    emit q->hostModeStateChanged(hostMode);
}

void QBluetoothLocalDevicePrivate::requestPairing(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing)
{
//#ifdef USING_BTENGCONNMAN
#ifdef USING_BTENGDEVMAN
    Q_Q(QBluetoothLocalDevice);

    BluetoothSymbianRegistryAdapter *registryAdapter = new BluetoothSymbianRegistryAdapter(address,q);
    // do nothing if everything is uptodate.
    if (registryAdapter->pairingStatus() == pairing) {
        _q_pairingFinished(address, pairing);
        return;
    }
    // pass q pointer so that adapter gets deleted when q object is deleted. Not optimal though.
    BluetoothSymbianPairingAdapter *pairingAdapter = new BluetoothSymbianPairingAdapter(address,q);

    // After pairing has completed we emit pairingFinished signal in every case.
    // earlier we checked pairing status from Symbian Bluetooth registry at this point but it
    // was not updated and the result was that we emitted wrong pairing status.
    QObject::connect(pairingAdapter, SIGNAL(pairingFinished(const QBluetoothAddress&,QBluetoothLocalDevice::Pairing)),
        q, SLOT(_q_pairingFinished(const QBluetoothAddress&,QBluetoothLocalDevice::Pairing)));
    // if pairing status changes then emit pairing finished.
    // Currently this is used after removing a paired device from Symbian Bluetooth registry
    QObject::connect(registryAdapter, SIGNAL(pairingStatusChanged(const QBluetoothAddress&,QBluetoothLocalDevice::Pairing)),
        q, SLOT(_q_pairingFinished(const QBluetoothAddress&,QBluetoothLocalDevice::Pairing)));
    // if there are pairing errors, emit them
    QObject::connect(registryAdapter, SIGNAL(registryHandlingError(int)),
        q, SLOT(_q_registryError(int)));
    QObject::connect(pairingAdapter, SIGNAL(pairingError(int)),
        q, SLOT(_q_pairingError(int)));

    switch (pairing) {
    case QBluetoothLocalDevice::Unpaired:
        // this is async method
        registryAdapter->removePairing();
        break;
    case QBluetoothLocalDevice::Paired:
    case QBluetoothLocalDevice::AuthorizedPaired:
        // this is async method
        // Note:  currently Symbian pairing function puts up a user dialog to confirm authorization;  
        // due to this, we cannot be sure if authorization is done or not, since it depends on user response
        pairingAdapter->startPairing(pairing);
        break;
    default:
        ASSERT(0);
        break;
    }
#endif //USING_BTENGDEVMAN
//#endif USING_BTENGCONNMAN
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevicePrivate::pairingStatus(const QBluetoothAddress &address) const
{
#ifdef USING_BTENGDEVMAN
    QScopedPointer<BluetoothSymbianRegistryAdapter> registryAdapter (new BluetoothSymbianRegistryAdapter(address));
    return registryAdapter->pairingStatus();
#endif //USING_BTENGDEVMAN
}

void QBluetoothLocalDevicePrivate::pairingConfirmation(bool confirmation)
{
}

void QBluetoothLocalDevicePrivate::_q_pairingFinished(const QBluetoothAddress &address,QBluetoothLocalDevice::Pairing pairing)
{
    Q_Q(QBluetoothLocalDevice);
    emit q->pairingFinished(address, pairing);
}
void QBluetoothLocalDevicePrivate::_q_registryError(int error)
{
    Q_Q(QBluetoothLocalDevice);
    //TODO Add more errorCodes to LocalDevice api and map them to Symbian errorcodes
    emit q->error(QBluetoothLocalDevice::UnknownError);
}
void QBluetoothLocalDevicePrivate::_q_pairingError(int error)
{
    Q_Q(QBluetoothLocalDevice);
    //TODO Add more errorCodes to LocalDevice api and map them to Symbian errorcodes
    emit q->error(QBluetoothLocalDevice::PairingError);
}

QBluetoothLocalDevice::QBluetoothLocalDevice(QObject *parent)
:   QObject(parent), d_ptr(new QBluetoothLocalDevicePrivate())
{
    d_ptr->q_ptr = this;
    if (this->d_ptr->m_settings == NULL) {
        delete this->d_ptr;
        this->d_ptr = NULL;
    }
}

QBluetoothLocalDevice::QBluetoothLocalDevice(const QBluetoothAddress &address, QObject *parent)
: QObject(parent), d_ptr(new QBluetoothLocalDevicePrivate())
{
    d_ptr->q_ptr = this;
    if (this->d_ptr->m_settings == NULL || address != this->d_ptr->address()) {
        delete this->d_ptr;
        this->d_ptr = NULL;
    }
}

QString QBluetoothLocalDevice::name() const
{
    if (!d_ptr)
        return QString();
    return d_ptr->name();
}

QBluetoothAddress QBluetoothLocalDevice::address() const
{
    if (!d_ptr)
        return QBluetoothAddress();
    return d_ptr->address();
}

void QBluetoothLocalDevice::powerOn()
{
    if (!d_ptr)
        return;
    d_ptr->powerOn();
}

void QBluetoothLocalDevice::setHostMode(QBluetoothLocalDevice::HostMode mode)
{
    if (!d_ptr)
        return;
    d_ptr->setHostMode(mode);
}

QBluetoothLocalDevice::HostMode QBluetoothLocalDevice::hostMode() const
{
    if (!d_ptr)
        return HostPoweredOff;
    return d_ptr->hostMode();
}

QList<QBluetoothHostInfo> QBluetoothLocalDevice::allDevices()
{
    QList<QBluetoothHostInfo> localDevices;
    QBluetoothHostInfo hostInfo;
    hostInfo.setName(QBluetoothLocalDevicePrivate::name());
    hostInfo.setAddress(QBluetoothLocalDevicePrivate::address());
    localDevices.append(hostInfo);
    return localDevices;
}

void QBluetoothLocalDevice::requestPairing(const QBluetoothAddress &address, Pairing pairing)
{
    if (!d_ptr)
        return;

    return d_ptr->requestPairing(address, pairing);
}

QBluetoothLocalDevice::Pairing QBluetoothLocalDevice::pairingStatus(const QBluetoothAddress &address) const
{
    if (!d_ptr)
        return QBluetoothLocalDevice::Unpaired;

    return d_ptr->pairingStatus(address);
}

void QBluetoothLocalDevice::pairingConfirmation(bool confirmation)
{
    if (!d_ptr)
        return;

    return d_ptr->pairingConfirmation(confirmation);
}

QTBLUETOOTH_END_NAMESPACE
