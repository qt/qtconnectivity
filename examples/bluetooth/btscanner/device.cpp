// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "device.h"
#include "service.h"
#include "ui_device.h"

#include <qbluetoothaddress.h>
#include <qbluetoothdevicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>

#include <QMenu>
#include <QDebug>

static QColor colorForPairing(QBluetoothLocalDevice::Pairing pairing)
{
    return pairing == QBluetoothLocalDevice::Paired
           || pairing == QBluetoothLocalDevice::AuthorizedPaired
           ? QColor(Qt::green) : QColor(Qt::red);
}

DeviceDiscoveryDialog::DeviceDiscoveryDialog(QWidget *parent) :
    QDialog(parent),
    localDevice(new QBluetoothLocalDevice),
    ui(new Ui_DeviceDiscovery)
{
    ui->setupUi(this);
    ui->stopScan->setVisible(false);

    // In case of multiple Bluetooth adapters it is possible to set the adapter
    // to be used. Example code:
    //
    // QBluetoothAddress address("XX:XX:XX:XX:XX:XX");
    // discoveryAgent = new QBluetoothDeviceDiscoveryAgent(address);

    discoveryAgent = new QBluetoothDeviceDiscoveryAgent();

    connect(ui->scan, &QAbstractButton::clicked, this, &DeviceDiscoveryDialog::startScan);
    connect(ui->stopScan, &QAbstractButton::clicked, this, &DeviceDiscoveryDialog::stopScan);

    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &DeviceDiscoveryDialog::addDevice);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &DeviceDiscoveryDialog::scanFinished);

    connect(ui->list, &QListWidget::itemActivated,
            this, &DeviceDiscoveryDialog::itemActivated);

    connect(localDevice, &QBluetoothLocalDevice::hostModeStateChanged,
            this, &DeviceDiscoveryDialog::hostModeStateChanged);

    hostModeStateChanged(localDevice->hostMode());
    // add context menu for devices to be able to pair device
    ui->list->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->list, &QWidget::customContextMenuRequested,
            this, &DeviceDiscoveryDialog::displayPairingMenu);
    connect(localDevice, &QBluetoothLocalDevice::pairingFinished,
            this, &DeviceDiscoveryDialog::pairingDone);
}

DeviceDiscoveryDialog::~DeviceDiscoveryDialog()
{
    delete discoveryAgent;
}

void DeviceDiscoveryDialog::addDevice(const QBluetoothDeviceInfo &info)
{
    const QString label = info.address().toString() + u' ' + info.name();
    const auto items = ui->list->findItems(label, Qt::MatchExactly);
    if (items.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(label);
        QBluetoothLocalDevice::Pairing pairingStatus = localDevice->pairingStatus(info.address());
        item->setForeground(colorForPairing(pairingStatus));
        ui->list->addItem(item);
    }
}

void DeviceDiscoveryDialog::startScan()
{
    discoveryAgent->start();
    ui->scan->setVisible(false);
    ui->stopScan->setVisible(true);
}

void DeviceDiscoveryDialog::stopScan()
{
    discoveryAgent->stop();
    scanFinished();
}

void DeviceDiscoveryDialog::scanFinished()
{
    ui->scan->setVisible(true);
    ui->stopScan->setVisible(false);
}

void DeviceDiscoveryDialog::itemActivated(QListWidgetItem *item)
{
    const QString text = item->text();
    const auto index = text.indexOf(' ');
    if (index == -1)
        return;

    QBluetoothAddress address(text.left(index));
    QString name(text.mid(index + 1));

    ServiceDiscoveryDialog d(name, address);
    d.exec();
}

void DeviceDiscoveryDialog::on_discoverable_clicked(bool clicked)
{
    if (clicked)
        localDevice->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    else
        localDevice->setHostMode(QBluetoothLocalDevice::HostConnectable);
}

void DeviceDiscoveryDialog::on_power_clicked(bool clicked)
{
    if (clicked)
        localDevice->powerOn();
    else
        localDevice->setHostMode(QBluetoothLocalDevice::HostPoweredOff);
}

void DeviceDiscoveryDialog::hostModeStateChanged(QBluetoothLocalDevice::HostMode mode)
{
    ui->power->setChecked(mode != QBluetoothLocalDevice::HostPoweredOff);
    ui->discoverable->setChecked(mode == QBluetoothLocalDevice::HostDiscoverable);

    const bool on = mode != QBluetoothLocalDevice::HostPoweredOff;
    ui->scan->setEnabled(on);
    ui->discoverable->setEnabled(on);
}
void DeviceDiscoveryDialog::displayPairingMenu(const QPoint &pos)
{
    if (ui->list->count() == 0)
        return;
    QMenu menu(this);
    QAction *pairAction = menu.addAction("Pair");
    QAction *removePairAction = menu.addAction("Remove Pairing");
    QAction *chosenAction = menu.exec(ui->list->viewport()->mapToGlobal(pos));
    QListWidgetItem *currentItem = ui->list->currentItem();

    QString text = currentItem->text();
    const auto index = text.indexOf(' ');
    if (index == -1)
        return;

    QBluetoothAddress address (text.left(index));
    if (chosenAction == pairAction) {
        localDevice->requestPairing(address, QBluetoothLocalDevice::Paired);
    } else if (chosenAction == removePairAction) {
        localDevice->requestPairing(address, QBluetoothLocalDevice::Unpaired);
    }
}
void DeviceDiscoveryDialog::pairingDone(const QBluetoothAddress &address,
                                        QBluetoothLocalDevice::Pairing pairing)
{
    const auto items = ui->list->findItems(address.toString(), Qt::MatchContains);
    const QColor color = colorForPairing(pairing);
    for (auto *item : items)
        item->setForeground(color);
}
