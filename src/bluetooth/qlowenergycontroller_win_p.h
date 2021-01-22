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

#ifndef QLOWENERGYCONTROLLERPRIVATE_WIN32_P_H
#define QLOWENERGYCONTROLLERPRIVATE_WIN32_P_H

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

#include <qglobal.h>
#include <QtCore/QQueue>
#include <QtCore/QVector>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include "qlowenergycontroller.h"
#include "qlowenergycontrollerbase_p.h"

#include <windows/qwinlowenergybluetooth_p.h>

QT_BEGIN_NAMESPACE

class QThread;
class QLowEnergyControllerPrivateWin32;

class ThreadWorkerJob
{
public:
     enum Operation { WriteChar, ReadChar, WriteDescr, ReadDescr };
     Operation operation;
     QVariant data;
};

struct WriteCharData
{
    QByteArray newValue;
    QLowEnergyService::WriteMode mode;
    HANDLE hService;
    DWORD flags;
    BTH_LE_GATT_CHARACTERISTIC gattCharacteristic;
    int systemErrorCode;
};

struct ReadCharData
{
    QByteArray value;
    HANDLE hService;
    BTH_LE_GATT_CHARACTERISTIC gattCharacteristic;
    int systemErrorCode;
};

struct WriteDescData
{
    QByteArray newValue;
    HANDLE hService;
    BTH_LE_GATT_DESCRIPTOR gattDescriptor;
    int systemErrorCode;
};

struct ReadDescData
{
    QByteArray value;
    HANDLE hService;
    BTH_LE_GATT_DESCRIPTOR gattDescriptor;
    int systemErrorCode;
};

class ThreadWorker : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE void putJob(const ThreadWorkerJob &job);
    Q_INVOKABLE void runPendingJob();
signals:
    void jobFinished(const ThreadWorkerJob &job);
private:
    QVector<ThreadWorkerJob> m_jobs;
};

class QLowEnergyServiceData;

extern void registerQLowEnergyControllerMetaType();

class QLowEnergyControllerPrivateWin32 : public QLowEnergyControllerPrivate
{
    Q_OBJECT
public:
    QLowEnergyControllerPrivateWin32();
    ~QLowEnergyControllerPrivateWin32();

    void init() override;

    void connectToDevice() override;
    void disconnectFromDevice() override;

    void discoverServices() override;
    void discoverServiceDetails(const QBluetoothUuid &service) override;

    void startAdvertising(const QLowEnergyAdvertisingParameters &params,
                          const QLowEnergyAdvertisingData &advertisingData,
                          const QLowEnergyAdvertisingData &scanResponseData) override;
    void stopAdvertising() override;

    void requestConnectionUpdate(const QLowEnergyConnectionParameters &params) override;

    // read data
    void readCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
                            const QLowEnergyHandle charHandle) override;
    void readDescriptor(const QSharedPointer<QLowEnergyServicePrivate> service,
                        const QLowEnergyHandle charHandle,
                        const QLowEnergyHandle descriptorHandle) override;

    // write data
    void writeCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
                             const QLowEnergyHandle charHandle,
                             const QByteArray &newValue, QLowEnergyService::WriteMode mode) override;
    void writeDescriptor(const QSharedPointer<QLowEnergyServicePrivate> service,
                         const QLowEnergyHandle charHandle,
                         const QLowEnergyHandle descriptorHandle,
                         const QByteArray &newValue) override;

    void addToGenericAttributeList(const QLowEnergyServiceData &service,
                                   QLowEnergyHandle startHandle) override;
public slots:
    void jobFinished(const ThreadWorkerJob &job);
protected:
    void customEvent(QEvent *e);
private:
    QThread *thread = nullptr;
    ThreadWorker *threadWorker = nullptr;
    QString deviceSystemPath;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(ThreadWorkerJob)
Q_DECLARE_METATYPE(WriteCharData)
Q_DECLARE_METATYPE(ReadCharData)
Q_DECLARE_METATYPE(WriteDescData)
Q_DECLARE_METATYPE(ReadDescData)

#endif // QLOWENERGYCONTROLLERPRIVATE_WIN32__P_H
