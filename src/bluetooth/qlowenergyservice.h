/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QLOWENERGYSERVICE_H
#define QLOWENERGYSERVICE_H

#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QLowEnergyCharacteristic>

QT_BEGIN_NAMESPACE

class QLowEnergyServicePrivate;
class QLowEnergyControllerNewPrivate;
class Q_BLUETOOTH_EXPORT QLowEnergyService : public QObject
{
    Q_OBJECT
public:
    enum ServiceType {
        PrimaryService = 0,
        IncludedService
    };

    enum ServiceError {
        NoError = 0,
        ServiceNotValidError
    };

    enum ServiceState {
        InvalidService = 0, // when underlying controller disconnects
        DiscoveryRequired,  // we know start/end handle but nothing more
        DiscoveringServices,// discoverDetails() called and running
        ServiceDiscovered,  // all details have been synchronized
    };

    ~QLowEnergyService();

    QList<QSharedPointer<QLowEnergyService> > includedServices() const;
    QLowEnergyService::ServiceType type() const;
    QLowEnergyService::ServiceState state() const;


    QList<QLowEnergyCharacteristic> characteristics() const;
    QBluetoothUuid serviceUuid() const;
    QString serviceName() const;

    void discoverDetails();

    ServiceError error() const;


Q_SIGNALS:
    void stateChanged(QLowEnergyService::ServiceState newState);
    void characteristicChanged(const QLowEnergyCharacteristic &info,
                               const QByteArray &value);
    void descriptorChanged(const QLowEnergyDescriptorInfo &info,
                           const QByteArray &value);
    void error(QLowEnergyService::ServiceError error);

private:
    Q_DECLARE_PRIVATE(QLowEnergyService)
    QSharedPointer<QLowEnergyServicePrivate> d_ptr;

    // QLowEnergyControllerNew is the factory for this class
    friend class QLowEnergyControllerNew;
    QLowEnergyService(QSharedPointer<QLowEnergyServicePrivate> p,
                      QObject *parent = 0);
};

QT_END_NAMESPACE

#endif // QLOWENERGYSERVICE_H
