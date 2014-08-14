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

#ifndef GATOPERIPHERAL_P_H
#define GATOPERIPHERAL_P_H

#include "gatoperipheral.h"
#include "gatoservice.h"
#include "gatocharacteristic.h"
#include "gatodescriptor.h"
#include "gatoattclient.h"

QT_BEGIN_NAMESPACE

class GatoPeripheralPrivate : public QObject
{
    Q_OBJECT

    Q_DECLARE_PUBLIC(GatoPeripheral)

public:
    GatoPeripheralPrivate(GatoPeripheral *parent);
    ~GatoPeripheralPrivate();

    GatoPeripheral *q_ptr;
    GatoAddress addr;
    QString name;
    QSet<GatoUUID> service_uuids;
    QMap<GatoHandle, GatoService> services;

    bool complete_name : 1;
    bool complete_services : 1;

    /** Maps attribute handles to service handles. */
    QMap<GatoHandle, GatoHandle> characteristic_to_service;
    QMap<GatoHandle, GatoHandle> value_to_characteristic;
    QMap<GatoHandle, GatoHandle> descriptor_to_characteristic;

    GatoAttClient *att;
    QMap<uint, GatoUUID> pending_primary_reqs;
    QMap<uint, GatoHandle> pending_characteristic_reqs;
    QMap<uint, GatoHandle> pending_characteristic_read_reqs;
    QMap<uint, GatoHandle> pending_descriptor_reqs;
    QMap<uint, GatoHandle> pending_descriptor_read_reqs;

    QMap<GatoHandle, bool> pending_set_notify;

    void parseEIRFlags(quint8 data[], int len);
    void parseEIRUUIDs(int size, bool complete, quint8 data[], int len);
    void parseName(bool complete, quint8 data[], int len);

    static GatoCharacteristic parseCharacteristicValue(const QByteArray &ba);

    static QByteArray genClientCharConfiguration(bool notification, bool indication);

    void clearServices();
    void clearServiceCharacteristics(GatoService *service);
    void clearCharacteristicDescriptors(GatoCharacteristic *characteristic);

    void finishSetNotifyOperations(const GatoCharacteristic &characteristic);

public slots:
    void handleAttConnected();
    void handleAttDisconnected();
    void handleAttAttributeUpdated(GatoHandle handle, const QByteArray &value, bool confirmed);
    void handlePrimary(uint req, const QList<GatoAttClient::AttributeGroupData>& list);
    void handlePrimaryForService(uint req, const QList<GatoAttClient::HandleInformation>& list);
    void handleCharacteristic(uint req, const QList<GatoAttClient::AttributeData> &list);
    void handleDescriptors(uint req, const QList<GatoAttClient::InformationData> &list);
    void handleCharacteristicRead(uint req, const QByteArray &value);
    void handleDescriptorRead(uint req, const QByteArray &value);
    void handleCharacteristicWrite(uint req, bool ok);
    void handleDescriptorWrite(uint req, bool ok);
};

QT_END_NAMESPACE

#endif // GATOPERIPHERAL_P_H
