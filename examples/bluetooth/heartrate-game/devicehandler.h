// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "bluetoothbaseclass.h"

#include <QDateTime>
#include <QList>
#include <QTimer>

#include <QLowEnergyController>
#include <QLowEnergyService>

class DeviceInfo;

class DeviceHandler : public BluetoothBaseClass
{
    Q_OBJECT

    Q_PROPERTY(bool measuring READ measuring NOTIFY measuringChanged)
    Q_PROPERTY(bool alive READ alive NOTIFY aliveChanged)
    Q_PROPERTY(int hr READ hr NOTIFY statsChanged)
    Q_PROPERTY(int maxHR READ maxHR NOTIFY statsChanged)
    Q_PROPERTY(int minHR READ minHR NOTIFY statsChanged)
    Q_PROPERTY(float average READ average NOTIFY statsChanged)
    Q_PROPERTY(int time READ time NOTIFY statsChanged)
    Q_PROPERTY(float calories READ calories NOTIFY statsChanged)
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)

public:
    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    DeviceHandler(QObject *parent = nullptr);

    void setDevice(DeviceInfo *device);
    void setAddressType(AddressType type);
    AddressType addressType() const;

    bool measuring() const;
    bool alive() const;

    // Statistics
    int hr() const;
    int time() const;
    float average() const;
    int maxHR() const;
    int minHR() const;
    float calories() const;

signals:
    void measuringChanged();
    void aliveChanged();
    void statsChanged();

public slots:
    void startMeasurement();
    void stopMeasurement();
    void disconnectService();

private:
    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void updateHeartRateValue(const QLowEnergyCharacteristic &c,
                              const QByteArray &value);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d,
                              const QByteArray &value);

#ifdef SIMULATOR
    void updateDemoHR();
#endif
private:
    void addMeasurement(int value);

    QLowEnergyController *m_control = nullptr;
    QLowEnergyService *m_service = nullptr;
    QLowEnergyDescriptor m_notificationDesc;
    DeviceInfo *m_currentDevice = nullptr;

    bool m_foundHeartRateService;
    bool m_measuring;
    int m_currentValue, m_min, m_max, m_sum;
    float m_avg, m_calories;

    // Statistics
    QDateTime m_start;
    QDateTime m_stop;

    QList<int> m_measurements;
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;

#ifdef SIMULATOR
    QTimer m_demoTimer;
#endif
};

#endif // DEVICEHANDLER_H
