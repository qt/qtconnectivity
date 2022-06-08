// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef BTLOCALDEVICE_H
#define BTLOCALDEVICE_H

#include <QtCore/QObject>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothServer>
#include <QtBluetooth/QBluetoothServiceDiscoveryAgent>
#include <QtBluetooth/QBluetoothSocket>

class BtLocalDevice : public QObject
{
    Q_OBJECT
public:
    explicit BtLocalDevice(QObject *parent = 0);
    ~BtLocalDevice();
    Q_PROPERTY(QString hostMode READ hostMode NOTIFY hostModeStateChanged)
    Q_PROPERTY(int secFlags READ secFlags WRITE setSecFlags
               NOTIFY secFlagsChanged)

    QBluetooth::SecurityFlags secFlags() const;
    void setSecFlags(int);
    QString hostMode() const;

signals:
    void errorOccurred(QBluetoothLocalDevice::Error error);
    void hostModeStateChanged();
    void socketStateUpdate(int foobar);
    void secFlagsChanged();

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
};

#endif // BTLOCALDEVICE_H
