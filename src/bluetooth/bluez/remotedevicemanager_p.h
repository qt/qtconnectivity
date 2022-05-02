/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef REMOTEDEVICEMANAGER_P_H
#define REMOTEDEVICEMANAGER_P_H

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

#include <deque>

#include <QList>
#include <QMutex>
#include <QObject>

#include <QtBluetooth/qbluetoothaddress.h>


QT_BEGIN_NAMESPACE

// This API is kept a bit more generic in anticipation of further changes in the future.

class RemoteDeviceManager : public QObject
{
    Q_OBJECT
public:
    enum class JobType
    {
        JobDisconnectDevice,
    };

    explicit RemoteDeviceManager(const QBluetoothAddress& localAddress, QObject *parent = nullptr);

    bool isJobInProgress() const { return jobInProgress; }
    bool scheduleJob(JobType job, const QList<QBluetoothAddress> &remoteDevices);

signals:
    void finished();

private slots:
    void runQueue();
    void prepareNextJob();

private:
    void disconnectDevice(const QBluetoothAddress& remote);

    bool jobInProgress = false;
    QBluetoothAddress localAddress;
    std::deque<std::pair<JobType, QBluetoothAddress>> jobQueue;
    QString adapterPath;
};

QT_END_NAMESPACE

#endif // REMOTEDEVICEMANAGER_P_H
