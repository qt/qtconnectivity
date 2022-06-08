// Copyright (C) 2014 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PINGPONG_H
#define PINGPONG_H

#include <QTimer>
#include <QObject>
#include <qbluetoothserver.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothservicediscoveryagent.h>

static QString serviceUuid(QStringLiteral("e8e10f95-1a70-4b27-9ccf-02010264e9c9"));
static QString androidUuid(QStringLiteral("c9e96402-0102-cf9c-274b-701a950fe1e8"));

class PingPong: public QObject
{
    Q_OBJECT
    Q_PROPERTY(float ballX READ ballX NOTIFY ballChanged)
    Q_PROPERTY(float ballY READ ballY NOTIFY ballChanged)
    Q_PROPERTY(float leftBlockY READ leftBlockY NOTIFY leftBlockChanged)
    Q_PROPERTY(float rightBlockY READ rightBlockY NOTIFY rightBlockChanged)
    Q_PROPERTY(bool showDialog READ showDialog NOTIFY showDialogChanged)
    Q_PROPERTY(QString message READ message NOTIFY showDialogChanged)
    Q_PROPERTY(int role READ role NOTIFY roleChanged)
    Q_PROPERTY(int leftResult READ leftResult NOTIFY resultChanged)
    Q_PROPERTY(int rightResult READ rightResult NOTIFY resultChanged)
public:
    PingPong();
    ~PingPong();
    float ballX() const;
    float ballY() const;
    float leftBlockY() const;
    float rightBlockY() const;
    void checkBoundaries();
    bool showDialog() const;
    QString message() const;
    void setMessage(const QString &message);
    int role() const;
    int leftResult() const;
    int rightResult() const;
    void checkResult();

public slots:
    void startGame();
    void update();
    void updateBall(const float &bX, const float &bY);
    void updateLeftBlock(const float &lY);
    void updateRightBlock(const float &rY);
    void startServer();
    void startClient();
    void clientConnected();
    void clientDisconnected();
    void serverConnected();
    void serverDisconnected();
    void socketError(QBluetoothSocket::SocketError);
    void serverError(QBluetoothServer::Error);
    void serviceScanError(QBluetoothServiceDiscoveryAgent::Error);
    void done();
    void addService(const QBluetoothServiceInfo &);
    void readSocket();

Q_SIGNALS:
    void ballChanged();
    void leftBlockChanged();
    void rightBlockChanged();
    void showDialogChanged();
    void roleChanged();
    void resultChanged();

private:
    QBluetoothServer *m_serverInfo = nullptr;
    QBluetoothServiceInfo m_serviceInfo;
    QBluetoothSocket *socket = nullptr;
    QBluetoothServiceDiscoveryAgent *discoveryAgent = nullptr;

    float m_ballX = 0.5f;
    float m_ballY = 0.9f;
    float m_leftBlockY = 0.4f;
    float m_rightBlockY = 0.4f;
    QTimer *m_timer = nullptr;
    float m_speedy = 0.002f;
    float m_speedx = 0.002f;
    int m_resultLeft = 0;
    int m_resultRight = 0;
    bool m_showDialog = false;
    QString m_message;
    int m_role = 0;
    bool m_serviceFound = false;
};

#endif // PINGPONG_H
