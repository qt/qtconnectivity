/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include "device.h"
#include <qbluetoothaddress.h>
#include <qbluetoothdevicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothserviceinfo.h>
#include <qlowenergycharacteristicinfo.h>
#include <qlowenergycontroller.h>
#include <QDebug>
#include <QList>

Device::Device():
    localDevice(new QBluetoothLocalDevice()), m_message(QString()), connected(false)
{
    if (localDevice->allDevices().size() == 0) {
        setUpdate("Bluetooth adapter not discovered");
        m_adapterPresent = false;
    }
    else
        m_adapterPresent = true;
    QBluetoothAddress adapterAddress(localDevice->address());
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(adapterAddress);
    connect(discoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
            this, SLOT(addDevice(const QBluetoothDeviceInfo&)));
    connect(discoveryAgent, SIGNAL(finished()), this, SLOT(scanFinished()));
    serviceDiscoveryAgent = new QBluetoothServiceDiscoveryAgent(adapterAddress);

    setUpdate("Search");
}

Device::~Device()
{
    delete discoveryAgent;
    delete localDevice;
    delete serviceDiscoveryAgent;
    qDeleteAll(devices);
    qDeleteAll(m_services);
    qDeleteAll(m_characteristics);
    devices.clear();
    m_services.clear();
    m_characteristics.clear();
}

void Device::startDeviceDiscovery()
{
    if (m_adapterPresent) {
        devices.clear();
        setUpdate("Scanning for devices ...");
        discoveryAgent->start();
    }
    else
        setUpdate("Bluetooth adapter not discovered");
}

void Device::addDevice(const QBluetoothDeviceInfo &info)
{
    if (info.coreConfiguration() == QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        DeviceInfo *d = new DeviceInfo(info);
        devices.append(d);
        setUpdate("Last device added: " + d->getName());
    }
}

void Device::scanFinished()
{
    Q_EMIT devicesDone();
    setUpdate("Done! Scan Again!");
}

QVariant Device::getDevices()
{
    return QVariant::fromValue(devices);
}

QVariant Device::getServices()
{
    return QVariant::fromValue(m_services);
}

QVariant Device::getCharacteristics()
{
    return QVariant::fromValue(m_characteristics);
}

QString Device::getUpdate()
{
    return m_message;
}

void Device::scanServices(QString address)
{
    // We need the current device for service discovery.
    for (int i = 0; i < devices.size(); i++) {
        if (((DeviceInfo*)devices.at(i))->getAddress() == address ) {
            currentDevice.setDevice(((DeviceInfo*)devices.at(i))->getDevice());
        }
    }

    m_services.clear();
    //Connecting signals and slots for service discovery
    QBluetoothDeviceInfo dev = currentDevice.getDevice();
    connect(serviceDiscoveryAgent, SIGNAL(serviceDiscovered(const QLowEnergyServiceInfo&)),
            this, SLOT(addLowEnergyService(const QLowEnergyServiceInfo&)));
    connect(serviceDiscoveryAgent, SIGNAL(finished()), this, SLOT(serviceScanDone()));
    serviceDiscoveryAgent->setRemoteAddress(dev.address());
    serviceDiscoveryAgent->start();
    setUpdate("Scanning for services...");

    // Connecting signals and slots for connecting to LE services.
    info = new QLowEnergyController();
    connect(info, SIGNAL(connected(QLowEnergyServiceInfo)), this, SLOT(serviceConnected(QLowEnergyServiceInfo)));
    connect(info, SIGNAL(error(QLowEnergyServiceInfo)), this, SLOT(errorReceived(QLowEnergyServiceInfo)));
    connect(info, SIGNAL(disconnected(QLowEnergyServiceInfo)), this, SLOT(serviceDisconnected(QLowEnergyServiceInfo)));
}

void Device::addLowEnergyService(const QLowEnergyServiceInfo &service)
{
    ServiceInfo *serv = new ServiceInfo(service);
    m_services.append(serv);
}

void Device::serviceScanDone()
{
    Q_EMIT servicesDone();
    setUpdate("Service scan done!");
}

void Device::connectToService(const QString &uuid)
{
    QString serviceUuid = uuid;
    serviceUuid = serviceUuid.remove(QLatin1Char('{')).remove(QLatin1Char('}'));
    QBluetoothUuid u(serviceUuid);
    qDebug() << u;
    QLowEnergyServiceInfo a;
    for (int i = 0; i < m_services.size(); i++) {
        ServiceInfo *service = (ServiceInfo*)m_services.at(i);
        a = QLowEnergyServiceInfo(service->getLeService());
        if (a.serviceUuid() == u)
            info->connectToService(a);
    }
}

void Device::serviceConnected(const QLowEnergyServiceInfo &service)
{
    m_characteristics.clear();
    setUpdate("Service connected!");
    connected = true;
    for (int i = 0; i < service.characteristics().size(); i++) {
        CharacteristicInfo *chars = new CharacteristicInfo((QLowEnergyCharacteristicInfo)service.characteristics().at(i));
        m_characteristics.append(chars);
    }
    emit characteristicsDone();
}

void Device::errorReceived(const QLowEnergyServiceInfo &service)
{
    qDebug() << "Error: " << info->errorString() << service.serviceUuid();
    setUpdate(info->errorString());
}

void Device::setUpdate(QString message)
{
    m_message = message;
    emit updateChanged();
}

void Device::disconnectFromService()
{
    if (connected)
        info->disconnectFromService();
}

void Device::serviceDisconnected(const QLowEnergyServiceInfo &service)
{
    setUpdate("Service Disconnected");
}
