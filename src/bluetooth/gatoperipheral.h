/****************************************************************************
**
** Copyright (C) 2013 Javier de San Pedro <dev.git@javispedro.com>
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

#ifndef GATOPERIPHERAL_H
#define GATOPERIPHERAL_H

#include <QtCore/QObject>
#include "libgato_global.h"
#include "gatouuid.h"
#include "gatoaddress.h"

QT_BEGIN_NAMESPACE

class GatoService;
class GatoCharacteristic;
class GatoDescriptor;
class GatoPeripheralPrivate;

class LIBGATO_EXPORT GatoPeripheral : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GatoPeripheral)
    Q_ENUMS(State)
    Q_ENUMS(WriteType)
    Q_PROPERTY(GatoAddress address READ address)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)

public:
    GatoPeripheral(const GatoAddress& addr, QObject *parent = 0);
    ~GatoPeripheral();

    enum State {
        StateDisconnected,
        StateConnecting,
        StateConnected
    };

    enum WriteType {
        WriteWithResponse = 0,
        WriteWithoutResponse
    };

    State state() const;
    GatoAddress address() const;
    QString name() const;
    QList<GatoService> services() const;

    void parseEIR(quint8 data[], int len);
    bool advertisesService(const GatoUUID &uuid) const;

public Q_SLOTS:
    void connectPeripheral();
    void disconnectPeripheral();
    void discoverServices();
    void discoverServices(const QList<GatoUUID>& serviceUUIDs);
    void discoverCharacteristics(const GatoService &service);
    void discoverCharacteristics(const GatoService &service, const QList<GatoUUID>& characteristicUUIDs);
    void discoverDescriptors(const GatoCharacteristic &characteristic);
    void readValue(const GatoCharacteristic &characteristic);
    void readValue(const GatoDescriptor &descriptor);
    void writeValue(const GatoCharacteristic &characteristic, const QByteArray &data, WriteType type = WriteWithResponse);
    void writeValue(const GatoDescriptor &descriptor, const QByteArray &data);
    void setNotification(const GatoCharacteristic &characteristic, bool enabled);

Q_SIGNALS:
    void connected();
    void disconnected();
    void nameChanged();
    void servicesDiscovered();
    void characteristicsDiscovered(const GatoService &service);
    void descriptorsDiscovered(const GatoCharacteristic &characteristic);
    void valueUpdated(const GatoCharacteristic &characteristic, const QByteArray &value);
    void descriptorValueUpdated(const GatoDescriptor &descriptor, const QByteArray &value);

private:
    GatoPeripheralPrivate *const d_ptr;
};

QT_END_NAMESPACE

#endif // GATOPERIPHERAL_H
