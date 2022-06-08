// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SERVICE_H
#define SERVICE_H


#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QBluetoothAddress)
QT_FORWARD_DECLARE_CLASS(QBluetoothServiceInfo)
QT_FORWARD_DECLARE_CLASS(QBluetoothServiceDiscoveryAgent)

QT_FORWARD_DECLARE_CLASS(Ui_ServiceDiscovery)

QT_USE_NAMESPACE

class ServiceDiscoveryDialog : public QDialog
{
    Q_OBJECT

public:
    ServiceDiscoveryDialog(const QString &name, const QBluetoothAddress &address, QWidget *parent = nullptr);
    ~ServiceDiscoveryDialog();

public slots:
    void addService(const QBluetoothServiceInfo&);

private:
    QBluetoothServiceDiscoveryAgent *discoveryAgent;
    Ui_ServiceDiscovery *ui;
};

#endif
