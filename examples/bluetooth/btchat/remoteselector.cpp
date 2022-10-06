// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "remoteselector.h"
#include "ui_remoteselector.h"

#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/qbluetoothservicediscoveryagent.h>

QT_USE_NAMESPACE

RemoteSelector::RemoteSelector(const QBluetoothAddress &localAdapter, QWidget *parent)
    :   QDialog(parent), ui(new Ui::RemoteSelector)
{
    ui->setupUi(this);

    m_discoveryAgent = new QBluetoothServiceDiscoveryAgent(localAdapter);

    connect(m_discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
            this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));
    connect(m_discoveryAgent, SIGNAL(finished()), this, SLOT(discoveryFinished()));
    connect(m_discoveryAgent, SIGNAL(canceled()), this, SLOT(discoveryFinished()));
}

RemoteSelector::~RemoteSelector()
{
    delete ui;
    delete m_discoveryAgent;
}

void RemoteSelector::startDiscovery(const QBluetoothUuid &uuid)
{
    ui->status->setText(tr("Scanning..."));
    if (m_discoveryAgent->isActive())
        m_discoveryAgent->stop();

    ui->remoteDevices->clear();

    m_discoveryAgent->setUuidFilter(uuid);
    m_discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);

}

void RemoteSelector::stopDiscovery()
{
    if (m_discoveryAgent){
        m_discoveryAgent->stop();
    }
}

QBluetoothServiceInfo RemoteSelector::service() const
{
    return m_service;
}

void RemoteSelector::serviceDiscovered(const QBluetoothServiceInfo &serviceInfo)
{
#if 0
    qDebug() << "Discovered service on"
             << serviceInfo.device().name() << serviceInfo.device().address().toString();
    qDebug() << "\tService name:" << serviceInfo.serviceName();
    qDebug() << "\tDescription:"
             << serviceInfo.attribute(QBluetoothServiceInfo::ServiceDescription).toString();
    qDebug() << "\tProvider:"
             << serviceInfo.attribute(QBluetoothServiceInfo::ServiceProvider).toString();
    qDebug() << "\tL2CAP protocol service multiplexer:"
             << serviceInfo.protocolServiceMultiplexer();
    qDebug() << "\tRFCOMM server channel:" << serviceInfo.serverChannel();
#endif
    const QBluetoothAddress address = serviceInfo.device().address();
    for (const QBluetoothServiceInfo &info : std::as_const(m_discoveredServices)) {
        if (info.device().address() == address)
            return;
    }

    QString remoteName;
    if (serviceInfo.device().name().isEmpty())
        remoteName = address.toString();
    else
        remoteName = serviceInfo.device().name();

    QListWidgetItem *item =
        new QListWidgetItem(QString::fromLatin1("%1 %2").arg(remoteName,
                                                             serviceInfo.serviceName()));

    m_discoveredServices.insert(item, serviceInfo);
    ui->remoteDevices->addItem(item);
}

void RemoteSelector::discoveryFinished()
{
    ui->status->setText(tr("Select the chat service to connect to."));
}

void RemoteSelector::on_remoteDevices_itemActivated(QListWidgetItem *item)
{
    m_service = m_discoveredServices.value(item);
    if (m_discoveryAgent->isActive())
        m_discoveryAgent->stop();

    accept();
}

void RemoteSelector::on_remoteDevices_itemClicked(QListWidgetItem *)
{
    ui->connectButton->setEnabled(true);
}

void RemoteSelector::on_connectButton_clicked()
{
    auto items = ui->remoteDevices->selectedItems();
    if (items.size()) {
        QListWidgetItem *item = items[0];
        m_service = m_discoveredServices.value(item);
        if (m_discoveryAgent->isActive())
            m_discoveryAgent->stop();

        accept();
    }
}

void RemoteSelector::on_cancelButton_clicked()
{
    reject();
}
