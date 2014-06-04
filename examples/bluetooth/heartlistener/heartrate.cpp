/***************************************************************************
**
** Copyright (C) 2014 BlackBerry Limited. All rights reserved.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "heartrate.h"

#include <qbluetoothaddress.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothlocaldevice.h>
#include <qlowenergyserviceinfo.h>
#include <qlowenergycharacteristicinfo.h>
#include <qbluetoothuuid.h>
#include <QTimer>

HeartRate::HeartRate():
    m_currentDevice(QBluetoothDeviceInfo()), foundHeartRateService(false), foundHeartRateCharacteristic(false), m_HRMeasurement(0), m_max(0), m_min(0), calories(0), m_leInfo(0), timer(0)
{
    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    m_serviceDiscoveryAgent = new QBluetoothServiceDiscoveryAgent(QBluetoothAddress());

    connect(m_deviceDiscoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
            this, SLOT(addDevice(const QBluetoothDeviceInfo&)));
    connect(m_deviceDiscoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(deviceScanError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(m_deviceDiscoveryAgent, SIGNAL(finished()), this, SLOT(scanFinished()));

    connect(m_serviceDiscoveryAgent, SIGNAL(serviceDiscovered(const QLowEnergyServiceInfo&)),
            this, SLOT(addLowEnergyService(const QLowEnergyServiceInfo&)));
    connect(m_serviceDiscoveryAgent, SIGNAL(finished()), this, SLOT(serviceScanDone()));
    connect(m_serviceDiscoveryAgent, SIGNAL(error(QBluetoothServiceDiscoveryAgent::Error)),
            this, SLOT(serviceScanError(QBluetoothServiceDiscoveryAgent::Error)));
}

HeartRate::~HeartRate()
{
    delete m_deviceDiscoveryAgent;
    delete m_serviceDiscoveryAgent;
    delete m_leInfo;
    delete timer;
    qDeleteAll(m_devices);
    m_devices.clear();
}

void HeartRate::deviceSearch()
{
    m_devices.clear();
    m_deviceDiscoveryAgent->start();
    setMessage("Scanning for devices...");
}

void HeartRate::addDevice(const QBluetoothDeviceInfo &device)
{
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        QBluetoothLocalDevice localDevice;
        QBluetoothLocalDevice::Pairing pairingStatus = localDevice.pairingStatus(device.address());
        if (pairingStatus == QBluetoothLocalDevice::Paired || pairingStatus == QBluetoothLocalDevice::AuthorizedPaired )
            qWarning() << "Discovered LE Device name: " << device.name() << " Address: " << device.address().toString() << " Paired";
        else
            qWarning() << "Discovered LE Device name: " << device.name() << " Address: " << device.address().toString() << " not Paired";
        DeviceInfo *dev = new DeviceInfo(device);
        m_devices.append(dev);
        setMessage("Low Energy device found. Scanning for more...");
    }
}

void HeartRate::scanFinished()
{
    if (m_devices.size() == 0)
        setMessage("No Low Energy devices found");
    Q_EMIT nameChanged();
}

void HeartRate::setMessage(QString message)
{
    m_info = message;
    Q_EMIT messageChanged();
}

QString HeartRate::message() const
{
    return m_info;
}

QVariant HeartRate::name()
{
    return QVariant::fromValue(m_devices);
}

void HeartRate::connectToService(const QString &address)
{
    bool deviceHere = false;
    for (int i = 0; i<m_devices.size(); i++) {
        if (((DeviceInfo*)m_devices.at(i))->getAddress() == address ) {
            m_currentDevice.setDevice(((DeviceInfo*)m_devices.at(i))->getDevice());
            setMessage("Device selected.");
            deviceHere = true;
        }
    }
    // This in case we are running demo mode
    if (!deviceHere)
        startDemo();
    else {
        QBluetoothDeviceInfo device = m_currentDevice.getDevice();
        //! [Connect signals]
        m_serviceDiscoveryAgent->setRemoteAddress(device.address());
        m_serviceDiscoveryAgent->start();
        if (!m_leInfo) {
            m_leInfo = new QLowEnergyController();
            connect(m_leInfo, SIGNAL(connected(QLowEnergyServiceInfo)), this, SLOT(serviceConnected(QLowEnergyServiceInfo)));
            connect(m_leInfo, SIGNAL(disconnected(QLowEnergyServiceInfo)), this, SLOT(serviceDisconnected(QLowEnergyServiceInfo)));
            connect(m_leInfo, SIGNAL(error(QLowEnergyServiceInfo,QLowEnergyController::Error)),
                    this, SLOT(errorReceived(QLowEnergyServiceInfo,QLowEnergyController::Error)));
            connect(m_leInfo, SIGNAL(error(QLowEnergyCharacteristicInfo,QLowEnergyController::Error)),
                    this, SLOT(errorReceivedCharacteristic(QLowEnergyCharacteristicInfo,QLowEnergyController::Error)));
            connect(m_leInfo, SIGNAL(valueChanged(QLowEnergyCharacteristicInfo)), this, SLOT(receiveMeasurement(QLowEnergyCharacteristicInfo)));
        }
        //! [Connect signals]
    }
}

void HeartRate::addLowEnergyService(const QLowEnergyServiceInfo &gatt)
{
    if (gatt.serviceUuid() == QBluetoothUuid::HeartRate) {
        setMessage("Heart Rate service discovered. Waiting for service scan to be done...");
        m_heartRateService = QLowEnergyServiceInfo(gatt);
        foundHeartRateService = true;
    }
}

//! [Connecting to service]
void HeartRate::serviceScanDone()
{
    //If HeartBelt is not connected (installed on the body) this message will stay.
    setMessage("Connecting to service... Be patient...");
    //It is not advisable to connect to BLE device right after scanning.
    if (foundHeartRateService)
        QTimer::singleShot(3000, this, SLOT(startConnection()));
    else
        setMessage("Heart Rate Service not found. Make sure your device is paired.");
}

void HeartRate::startConnection()
{
    // HeartRate belt that this application was using had a random device address. This is only needed
    // for Linux platform.
    // m_heartRateService.setRandomAddress();
    m_leInfo->connectToService(m_heartRateService);
}

void HeartRate::serviceConnected(const QLowEnergyServiceInfo &leService)
{
    setMessage("Connected to service. Waiting for updates");
    if (leService.serviceUuid() == QBluetoothUuid::HeartRate) {
        for ( int i = 0; i<leService.characteristics().size(); i++) {
            if (leService.characteristics().at(i).uuid() == QBluetoothUuid::HeartRateMeasurement) {
                m_start = QDateTime::currentDateTime();
                m_heartRateCharacteristic = QLowEnergyCharacteristicInfo(leService.characteristics().at(i));
                m_leInfo->enableNotifications(m_heartRateCharacteristic);
                foundHeartRateCharacteristic = true;
            }
        }
    }
}
//! [Connecting to service]

void HeartRate::receiveMeasurement(const QLowEnergyCharacteristicInfo &characteristic)
{
    m_heartRateCharacteristic = QLowEnergyCharacteristicInfo(characteristic);
    //! [Reading value]
    QString val;
    qint16 energy_expended = 0;
    int flags = 0;
    int index = 0;

    //8 bit flags
    val[0] = m_heartRateCharacteristic.value().at(index++);
    val[1] = m_heartRateCharacteristic.value().at(index++);
    flags = val.toUInt(0, 16);

    // Each Heart Belt has its own settings and features, besides heart rate measurement
    // By checking the flags we can determine whether it has energy feature. We will go through the array
    // of the characters in the characteristic value.
    // The following flags are used to determine what kind of value is being sent from the device.
    // see https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.heart_rate_measurement.xml
    quint8 heartRateValueIs16BitFlag = 0x1; //8 bit or 16 bit
    quint8 energyExpendedFeatureFlag = 0x8;

    val.clear();
    val[0] = m_heartRateCharacteristic.value().at(index++); //1st 4 bit
    val[1] = m_heartRateCharacteristic.value().at(index++); //2nd 4 bit
    if (flags & heartRateValueIs16BitFlag) { //16 bit value
        val[2] = m_heartRateCharacteristic.value().at(index++);
        val[3] = m_heartRateCharacteristic.value().at(index++);
    }

    m_HRMeasurement = val.toUInt(0, 16);
    //! [Reading value]
    m_measurements.append(m_HRMeasurement);

    if (flags & energyExpendedFeatureFlag) {
        QString energy;
        int counter1 = 0;
        for (int i = index; i < (m_heartRateCharacteristic.value().size() - 1); i++) {
            if (counter1 > 3)
                break;
            energy[i] = m_heartRateCharacteristic.value().at(i);
            counter1 ++;
        }
        energy_expended = energy.toUInt(0, 16);
    }
    qWarning() << "Used energy: " << energy_expended;
    Q_EMIT hrChanged();
}

int HeartRate::hR() const
{
    return m_HRMeasurement;
}

//! [Error handling]
void HeartRate::errorReceived(const QLowEnergyServiceInfo &leService, QLowEnergyController::Error error)
{
    qWarning() << "Error: " << leService.serviceUuid() << m_leInfo->errorString() << error;
    setMessage(QStringLiteral("Error: ") + m_leInfo->errorString());
}

void HeartRate::errorReceivedCharacteristic(const QLowEnergyCharacteristicInfo &leCharacteristic, QLowEnergyController::Error error)
{
    qWarning() << "Error: " << leCharacteristic.uuid() << m_leInfo->errorString() << error;
    setMessage(QStringLiteral("Error: ") + m_leInfo->errorString());
}
//! [Error handling]

void HeartRate::disconnectService()
{
    if (foundHeartRateCharacteristic) {
        m_stop = QDateTime::currentDateTime();
        //! [Disconnecting from service]
        m_leInfo->disableNotifications(m_heartRateCharacteristic);
        m_leInfo->disconnectFromService();
        //! [Disconnecting from service]
    }
    else if (m_devices.size() == 0) {
        m_stop = QDateTime::currentDateTime();
        timer->stop();
        timer = 0;
    }
    foundHeartRateCharacteristic = false;
    foundHeartRateService = false;
}

void HeartRate::obtainResults()
{
    Q_EMIT timeChanged();
    Q_EMIT averageChanged();
    Q_EMIT caloriesChanged();
}

int HeartRate::time()
{
    return m_start.secsTo(m_stop);
}

int HeartRate::maxHR() const
{
    return m_max;
}

int HeartRate::minHR() const
{
    return m_min;
}

float HeartRate::average()
{
    if (m_measurements.size() == 0)
        return 0;
    else {
        m_max = 0;
        m_min = 1000;
        int sum = 0;
        for (int i=0; i< m_measurements.size(); i++) {
            sum += (int) m_measurements.value(i);
            if (((int)m_measurements.value(i)) > m_max)
                m_max = (int)m_measurements.value(i);
            if (((int)m_measurements.value(i)) < m_min)
                m_min = (int)m_measurements.value(i);
        }
        return sum/m_measurements.size();
    }
}

int HeartRate::measurements(int index)
{
    if (index > m_measurements.size())
        return 0;
    else
        return (int)m_measurements.value(index);
}

int HeartRate::measurementsSize()
{
    return m_measurements.size();
}

QString HeartRate::deviceAddress()
{
    return m_currentDevice.getDevice().address().toString();
}

float HeartRate::caloriesCalculation()
{
    calories = ((-55.0969 + (0.6309 * average()) + (0.1988 * 94) + (0.2017 * 24)) / 4.184) * 60 * time()/3600 ;
    return calories;
}

void HeartRate::serviceDisconnected(const QLowEnergyServiceInfo &service)
{
    setMessage("Heart Rate service disconnected");
    qWarning() << "Service disconnected: " << service.serviceUuid();
}

int HeartRate::numDevices() const
{
    return m_devices.size();
}

void HeartRate::startDemo()
{
    m_start = QDateTime::currentDateTime();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(receiveDemo()));
    timer->start(1000);
    setMessage("This is Demo mode");
}

void HeartRate::receiveDemo()
{
    m_HRMeasurement = 60;
    m_measurements.append(m_HRMeasurement);
    Q_EMIT hrChanged();
}

void HeartRate::serviceScanError(QBluetoothServiceDiscoveryAgent::Error error)
{
    if (error == QBluetoothServiceDiscoveryAgent::PoweredOffError)
        setMessage("The Bluetooth adaptor is powered off, power it on before doing discovery.");
    else if (error == QBluetoothServiceDiscoveryAgent::InputOutputError)
        setMessage("Writing or reading from the device resulted in an error.");
    else
        setMessage("An unknown error has occurred.");
}

void HeartRate::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        setMessage("The Bluetooth adaptor is powered off, power it on before doing discovery.");
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        setMessage("Writing or reading from the device resulted in an error.");
    else
        setMessage("An unknown error has occurred.");
}
