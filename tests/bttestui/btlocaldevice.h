/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef BTLOCALDEVICE_H
#define BTLOCALDEVICE_H

#include <QtCore/QObject>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothServer>
#include <QtBluetooth/QBluetoothServiceDiscoveryAgent>
#include <QtBluetooth/QBluetoothSocket>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyServiceData>

class BtLocalDevice : public QObject
{
    Q_OBJECT
public:
    explicit BtLocalDevice(QObject *parent = 0);
    ~BtLocalDevice();
    Q_PROPERTY(QString hostMode READ hostMode NOTIFY hostModeStateChanged)
    Q_PROPERTY(int secFlags READ secFlags WRITE setSecFlags NOTIFY secFlagsChanged)
    Q_PROPERTY(bool centralExists READ centralExists NOTIFY leChanged);
    Q_PROPERTY(bool centralSubscribed READ centralSubscribed NOTIFY leChanged);
    Q_PROPERTY(QByteArray centralState READ centralState NOTIFY leChanged);
    Q_PROPERTY(QByteArray centralError READ centralError NOTIFY leChanged);
    Q_PROPERTY(QByteArray centralServiceState READ centralServiceState NOTIFY leChanged);
    Q_PROPERTY(QByteArray centralServiceError READ centralServiceError NOTIFY leChanged);
    Q_PROPERTY(QByteArray peripheralState READ peripheralState NOTIFY leChanged);
    Q_PROPERTY(QByteArray peripheralError READ peripheralError NOTIFY leChanged);
    Q_PROPERTY(QByteArray peripheralServiceState READ peripheralServiceState NOTIFY leChanged);
    Q_PROPERTY(QByteArray peripheralServiceError READ peripheralServiceError NOTIFY leChanged);
    Q_PROPERTY(bool peripheralExists READ peripheralExists NOTIFY leChanged);

    QBluetooth::SecurityFlags secFlags() const;
    void setSecFlags(int);
    QString hostMode() const;

signals:
    void errorOccurred(QBluetoothLocalDevice::Error error);
    void hostModeStateChanged();
    void socketStateUpdate(int foobar);
    void secFlagsChanged();
    bool leChanged(); // Same signal used for LE changes for simplicity

public slots:
    //QBluetoothLocalDevice
    void dumpInformation();
    void powerOn();
    void reset();
    void setHostMode(int newMode);
    void requestPairingUpdate(bool isPairing);
    void pairingFinished(const QBluetoothAddress &address, QBluetoothLocalDevice::Pairing pairing);
    void connected(const QBluetoothAddress &addr);
    void disconnected(const QBluetoothAddress &addr);
    void cycleSecurityFlags();

    //QBluetoothDeviceDiscoveryAgent
    void deviceDiscovered(const QBluetoothDeviceInfo &info);
    void discoveryFinished();
    void discoveryCanceled();
    void discoveryError(QBluetoothDeviceDiscoveryAgent::Error error);
    void startDiscovery();
    void stopDiscovery();

    //QBluetoothServiceDiscoveryAgent
    void startServiceDiscovery(bool isMinimalDiscovery);
    void startTargettedServiceDiscovery();
    void stopServiceDiscovery();
    void serviceDiscovered(const QBluetoothServiceInfo &info);
    void serviceDiscoveryFinished();
    void serviceDiscoveryCanceled();
    void serviceDiscoveryError(QBluetoothServiceDiscoveryAgent::Error error);
    void dumpServiceDiscovery();

    //QBluetoothSocket
    void connectToService();
    void connectToServiceViaSearch();
    void disconnectToService();
    void closeSocket();
    void abortSocket();
    void socketConnected();
    void socketDisconnected();
    void socketError(QBluetoothSocket::SocketError error);
    void socketStateChanged(QBluetoothSocket::SocketState);
    void dumpSocketInformation();
    void writeData();
    void readData();

    //QBluetoothServer
    void serverError(QBluetoothServer::Error error);
    void serverListenPort();
    void serverListenUuid();
    void serverClose();
    void serverNewConnection();
    void clientSocketDisconnected();
    void clientSocketReadyRead();
    void dumpServerInformation();

    //QLowEnergyController central
    void centralCreate();
    void centralConnect();
    void centralStartServiceDiscovery();
    void centralDiscoverServiceDetails();
    void centralWrite();
    void centralRead();
    void centralSubscribeUnsubscribe();
    void centralDelete();
    bool centralExists() const;
    bool centralSubscribed() const;
    QByteArray centralState() const;
    QByteArray centralServiceState() const;
    QByteArray centralError() const;
    QByteArray centralServiceError() const;

    //QLowEnergyController peripheral
    void peripheralCreate();
    void peripheralStartAdvertising();
    void peripheralStopAdvertising();
    void peripheralWrite();
    void peripheralRead();
    void peripheralDelete();
    bool peripheralExists() const;
    QByteArray peripheralState() const;
    QByteArray peripheralServiceState() const;
    QByteArray peripheralError() const;
    QByteArray peripheralServiceError() const;

    // QLowEnergyController misc
    void startLeDeviceDiscovery();
    void dumpLeInfo();

private:
    void dumpLocalDevice(QBluetoothLocalDevice *dev);

    QBluetoothLocalDevice *localDevice = nullptr;
    QBluetoothDeviceDiscoveryAgent *deviceAgent = nullptr;
    QBluetoothServiceDiscoveryAgent *serviceAgent = nullptr;
    QBluetoothSocket *socket = nullptr;
    QBluetoothServer *server = nullptr;
    QList<QBluetoothSocket *> serverSockets;
    QBluetoothServiceInfo serviceInfo;
    QList<QBluetoothServiceInfo> foundTestServers;
    QBluetooth::SecurityFlags securityFlags;

    std::unique_ptr<QLowEnergyController> leCentralController;
    std::unique_ptr<QLowEnergyController> lePeripheralController;
    std::unique_ptr<QLowEnergyService> lePeripheralService;
    std::unique_ptr<QLowEnergyService> leCentralService;
    QLowEnergyAdvertisingData leAdvertisingData;
    QLowEnergyServiceData leServiceData;
    QBluetoothDeviceInfo leRemotePeripheralDevice;
};

#endif // BTLOCALDEVICE_H
