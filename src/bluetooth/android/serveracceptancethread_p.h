/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef SERVERACCEPTANCETHREAD_H
#define SERVERACCEPTANCETHREAD_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QMutex>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtBluetooth/QBluetoothServer>
#include <QtBluetooth/QBluetoothUuid>
#include "qbluetooth.h"


class ServerAcceptanceThread : public QObject
{
    Q_OBJECT
public:
    explicit ServerAcceptanceThread(QObject *parent = nullptr);
    ~ServerAcceptanceThread();
    void setServiceDetails(const QBluetoothUuid &uuid, const QString &serviceName,
                           QBluetooth::SecurityFlags securityFlags);


    bool hasPendingConnections() const;
    QAndroidJniObject nextPendingConnection();
    void setMaxPendingConnections(int maximumCount);

    void javaThreadErrorOccurred(int errorCode);
    void javaNewSocket(jobject socket);

    void run();
    void stop();
    bool isRunning() const;

signals:
    void newConnection();
    void error(QBluetoothServer::Error);

private:
    bool validSetup() const;
    void shutdownPendingConnections();

    QList<QAndroidJniObject> pendingSockets;
    mutable QMutex m_mutex;
    QString m_serviceName;
    QBluetoothUuid m_uuid;
    int maxPendingConnections;
    QBluetooth::SecurityFlags secFlags;

    QAndroidJniObject javaThread;

};

#endif // SERVERACCEPTANCETHREAD_H
