// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DEVICE_H
#define DEVICE_H

#include <qbluetoothlocaldevice.h>

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QBluetoothDeviceDiscoveryAgent)
QT_FORWARD_DECLARE_CLASS(QBluetoothDeviceInfo)
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)

QT_FORWARD_DECLARE_CLASS(Ui_DeviceDiscovery)

QT_USE_NAMESPACE

class DeviceDiscoveryDialog : public QDialog
{
    Q_OBJECT

public:
    DeviceDiscoveryDialog(QWidget *parent = nullptr);
    ~DeviceDiscoveryDialog();

public slots:
    void addDevice(const QBluetoothDeviceInfo&);
    void on_power_clicked(bool clicked);
    void on_discoverable_clicked(bool clicked);
    void displayPairingMenu(const QPoint &pos);
    void pairingDone(const QBluetoothAddress&, QBluetoothLocalDevice::Pairing);
private slots:
    void startScan();
    void stopScan();
    void scanFinished();
    void itemActivated(QListWidgetItem *item);
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode);

private:
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    QBluetoothLocalDevice *localDevice;
    Ui_DeviceDiscovery *ui;
};

#endif
