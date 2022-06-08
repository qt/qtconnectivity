// Copyright (C) 2021 The Qt Company Ltd.
// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "service.h"
#include "ui_service.h"

#include <qbluetoothaddress.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothuuid.h>


ServiceDiscoveryDialog::ServiceDiscoveryDialog(const QString &name,
                                               const QBluetoothAddress &address, QWidget *parent)
:   QDialog(parent), ui(new Ui_ServiceDiscovery)
{
    ui->setupUi(this);

    //Using default Bluetooth adapter
    QBluetoothLocalDevice localDevice;
    QBluetoothAddress adapterAddress = localDevice.address();

    // In case of multiple Bluetooth adapters it is possible to
    // set which adapter will be used by providing MAC Address.
    // Example code:
    //
    // QBluetoothAddress adapterAddress("XX:XX:XX:XX:XX:XX");
    // discoveryAgent = new QBluetoothServiceDiscoveryAgent(adapterAddress);

    discoveryAgent = new QBluetoothServiceDiscoveryAgent(adapterAddress);

    discoveryAgent->setRemoteAddress(address);

    setWindowTitle(name);

    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
            this, &ServiceDiscoveryDialog::addService);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished,
            ui->status, &QWidget::hide);

    discoveryAgent->start();
}

ServiceDiscoveryDialog::~ServiceDiscoveryDialog()
{
    delete discoveryAgent;
    delete ui;
}

void ServiceDiscoveryDialog::addService(const QBluetoothServiceInfo &info)
{
    if (info.serviceName().isEmpty())
        return;

    QString line = info.serviceName();
    if (!info.serviceDescription().isEmpty())
        line.append("\n\t" + info.serviceDescription());
    if (!info.serviceProvider().isEmpty())
        line.append("\n\t" + info.serviceProvider());

    ui->list->addItem(line);
}
