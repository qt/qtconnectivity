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

#include <QtCore/QDebug>

#include <assert.h>
#include <bluetooth/bluetooth.h>

#include "gatoperipheral_p.h"
#include "gatoaddress.h"
#include "gatouuid.h"
#include "helpers.h"

QT_BEGIN_NAMESPACE

enum EIRDataFields {
    EIRFlags = 0x01,
    EIRIncompleteUUID16List = 0x02,
    EIRCompleteUUID16List = 0x03,
    EIRIncompleteUUID32List = 0x04,
    EIRCompleteUUID32List = 0x05,
    EIRIncompleteUUID128List = 0x06,
    EIRCompleteUUID128List = 0x07,
    EIRIncompleteLocalName = 0x08,
    EIRCompleteLocalName = 0x09,
    EIRTxPowerLevel = 0x0A,
    EIRDeviceClass = 0x0D,
    EIRSecurityManagerTKValue = 0x10,
    EIRSecurityManagerOutOfBandFlags = 0x11,
    EIRSolicitedUUID128List = 0x15
};

GatoPeripheral::GatoPeripheral(const GatoAddress &addr, QObject *parent) :
    QObject(parent), d_ptr(new GatoPeripheralPrivate(this))
{
    Q_D(GatoPeripheral);
    d->addr = addr;
    d->att = new GatoAttClient(this);

    connect(d->att, SIGNAL(connected()), d, SLOT(handleAttConnected()));
    connect(d->att, SIGNAL(disconnected()), d, SLOT(handleAttDisconnected()));
    connect(d->att, SIGNAL(attributeUpdated(GatoHandle,QByteArray,bool)), d, SLOT(handleAttAttributeUpdated(GatoHandle,QByteArray,bool)));
}

GatoPeripheral::~GatoPeripheral()
{
    if (state() != StateDisconnected) {
        disconnect();
    }
    delete d_ptr;
}

GatoPeripheral::State GatoPeripheral::state() const
{
    Q_D(const GatoPeripheral);
    return static_cast<State>(d->att->state());
}

GatoAddress GatoPeripheral::address() const
{
    Q_D(const GatoPeripheral);
    return d->addr;
}

QString GatoPeripheral::name() const
{
    Q_D(const GatoPeripheral);
    return d->name;
}

QList<GatoService> GatoPeripheral::services() const
{
    Q_D(const GatoPeripheral);
    return d->services.values();
}

void GatoPeripheral::parseEIR(quint8 data[], int len)
{
    Q_D(GatoPeripheral);

    int pos = 0;
    while (pos < len) {
        int item_len = data[pos];
        pos++;
        if (item_len == 0) break;
        int type = data[pos];
        assert(pos + item_len <= len);
        switch (type) {
        case EIRFlags:
            d->parseEIRFlags(&data[pos + 1], item_len - 1);
            break;
        case EIRIncompleteUUID16List:
            d->parseEIRUUIDs(16/8, false, &data[pos + 1], item_len - 1);
            break;
        case EIRCompleteUUID16List:
            d->parseEIRUUIDs(16/8, true, &data[pos + 1], item_len - 1);
            break;
        case EIRIncompleteUUID32List:
            d->parseEIRUUIDs(32/8, false, &data[pos + 1], item_len - 1);
            break;
        case EIRCompleteUUID32List:
            d->parseEIRUUIDs(32/8, true, &data[pos + 1], item_len - 1);
            break;
        case EIRIncompleteUUID128List:
            d->parseEIRUUIDs(128/8, false, &data[pos + 1], item_len - 1);
            break;
        case EIRCompleteUUID128List:
            d->parseEIRUUIDs(128/8, true, &data[pos + 1], item_len - 1);
            break;
        case EIRIncompleteLocalName:
            d->parseName(false, &data[pos + 1], item_len - 1);
            break;
        case EIRCompleteLocalName:
            d->parseName(true, &data[pos + 1], item_len - 1);
            break;
        case EIRTxPowerLevel:
        case EIRSolicitedUUID128List:
            qDebug() << "Unhandled EIR data type" << type;
            break;
        default:
            qWarning() << "Unknown EIR data type" << type;
            break;
        }

        pos += item_len;
    }

    assert(pos == len);
}

bool GatoPeripheral::advertisesService(const GatoUUID &uuid) const
{
    Q_D(const GatoPeripheral);
    return d->service_uuids.contains(uuid);
}

void GatoPeripheral::connectPeripheral()
{
    Q_D(GatoPeripheral);
    if (d->att->state() != GatoSocket::StateDisconnected) {
        qDebug() << "Already connecting";
        return;
    }

    d->att->connectTo(d->addr);
}

void GatoPeripheral::disconnectPeripheral()
{
    Q_D(GatoPeripheral);

    d->att->close();
}

void GatoPeripheral::discoverServices()
{
    Q_D(GatoPeripheral);
    if (!d->complete_services && state() == StateConnected) {
        d->clearServices();
        d->att->requestReadByGroupType(0x0001, 0xFFFF, GatoUUID::GattPrimaryService,
                                       d, SLOT(handlePrimary(QList<GatoAttClient::AttributeGroupData>)));
    } else {
        qWarning() << "Not connected";
    }
}

void GatoPeripheral::discoverServices(const QList<GatoUUID> &serviceUUIDs)
{
    Q_D(GatoPeripheral);
    if (serviceUUIDs.isEmpty()) return;
    if (state() == StateConnected) {
        foreach (const GatoUUID& uuid, serviceUUIDs) {
            QByteArray value = gatouuid_to_bytearray(uuid, true, false);
            uint req = d->att->requestFindByTypeValue(0x0001, 0xFFFF, GatoUUID::GattPrimaryService, value,
                                                      d, SLOT(handlePrimaryForService(uint,QList<GatoAttClient::HandleInformation>)));
            d->pending_primary_reqs.insert(req, uuid);
        }
    } else {
        qWarning() << "Not connected";
    }
}

void GatoPeripheral::discoverCharacteristics(const GatoService &service)
{
    Q_D(GatoPeripheral);

    if (!d->services.contains(service.startHandle())) {
        qWarning() << "Unknown service for this peripheral";
        return;
    }

    GatoService &our_service = d->services[service.startHandle()];

    if (our_service.startHandle() != service.startHandle() ||
            our_service.endHandle() != service.endHandle() ||
            our_service.uuid() != service.uuid()) {
        qWarning() << "Unknown service for this peripheral";
        return;
    }

    if (state() == StateConnected) {
        GatoHandle start = our_service.startHandle();
        GatoHandle end = our_service.endHandle();

        d->clearServiceCharacteristics(&our_service);

        uint req = d->att->requestReadByType(start, end, GatoUUID::GattCharacteristic,
                                             d, SLOT(handleCharacteristic(QList<GatoAttClient::AttributeData>)));
        d->pending_characteristic_reqs.insert(req, start);
    } else {
        qWarning() << "Not connected";
    }
}

void GatoPeripheral::discoverCharacteristics(const GatoService &service, const QList<GatoUUID> &characteristicUUIDs)
{
    // TODO There seems to be no way to ask for the peripheral to filter by uuid
    Q_UNUSED(characteristicUUIDs);
    discoverCharacteristics(service);
}

void GatoPeripheral::discoverDescriptors(const GatoCharacteristic &characteristic)
{
    Q_D(GatoPeripheral);

    GatoHandle char_handle = characteristic.startHandle();
    GatoHandle service_handle = d->characteristic_to_service.value(char_handle);

    if (!service_handle) {
        qWarning() << "Unknown characteristic for this peripheral";
        return;
    }

    GatoService &our_service = d->services[service_handle];
    Q_ASSERT(our_service.containsCharacteristic(char_handle));
    GatoCharacteristic our_char = our_service.getCharacteristic(char_handle);
    Q_ASSERT(our_char.startHandle() == char_handle);

    if (state() == StateConnected) {
        d->clearCharacteristicDescriptors(&our_char);
        our_service.addCharacteristic(our_char); // Update service with empty descriptors list
        uint req = d->att->requestFindInformation(our_char.startHandle() + 1, our_char.endHandle(),
                                                  d, SLOT(handleDescriptors(uint,QList<GatoAttClient::InformationData>)));
        d->pending_descriptor_reqs.insert(req, char_handle);
    } else {
        qWarning() << "Not connected";
    }
}

void GatoPeripheral::readValue(const GatoCharacteristic &characteristic)
{
    Q_D(GatoPeripheral);

    GatoHandle char_handle = characteristic.startHandle();
    GatoHandle service_handle = d->characteristic_to_service.value(char_handle);

    if (!service_handle) {
        qWarning() << "Unknown characteristic for this peripheral";
        return;
    }

    GatoService &our_service = d->services[service_handle];
    Q_ASSERT(our_service.containsCharacteristic(char_handle));

    if (state() == StateConnected) {
        uint req = d->att->requestRead(characteristic.valueHandle(),
                                       d, SLOT(handleCharacteristicRead(uint,QByteArray)));
        d->pending_characteristic_read_reqs.insert(req, char_handle);
    } else {
        qWarning() << "Not connected";
    }
}

void GatoPeripheral::readValue(const GatoDescriptor &descriptor)
{
    Q_D(GatoPeripheral);

    GatoHandle desc_handle = descriptor.handle();
    GatoHandle char_handle = d->descriptor_to_characteristic.value(desc_handle);

    if (!char_handle) {
        qWarning() << "Unknown descriptor for this peripheral";
        return;
    }

    GatoHandle service_handle = d->characteristic_to_service.value(char_handle);
    Q_ASSERT(service_handle);

    GatoService &our_service = d->services[service_handle];
    Q_ASSERT(our_service.containsCharacteristic(char_handle));

    if (state() == StateConnected) {
        uint req = d->att->requestRead(descriptor.handle(),
                                       d, SLOT(handleDescriptorRead(uint,QByteArray)));
        d->pending_descriptor_read_reqs.insert(req, char_handle);
    } else {
        qWarning() << "Not connected";
    }
}

void GatoPeripheral::writeValue(const GatoCharacteristic &characteristic, const QByteArray &data, WriteType type)
{
    Q_D(GatoPeripheral);

    GatoHandle char_handle = characteristic.startHandle();
    GatoHandle service_handle = d->characteristic_to_service.value(char_handle);

    if (!service_handle) {
        qWarning() << "Unknown characteristic for this peripheral";
        return;
    }

    GatoService &our_service = d->services[service_handle];
    Q_ASSERT(our_service.containsCharacteristic(char_handle));

    if (state() == StateConnected) {
        switch (type) {
        case WriteWithResponse:
            d->att->requestWrite(characteristic.valueHandle(), data,
                                 d, SLOT(handleCharacteristicWrite(uint,bool)));
            break;
        case WriteWithoutResponse:
            d->att->commandWrite(characteristic.valueHandle(), data);
            break;
        }


    } else {
        qWarning() << "Not connected";
    }
}

void GatoPeripheral::writeValue(const GatoDescriptor &descriptor, const QByteArray &data)
{
    Q_D(GatoPeripheral);

    GatoHandle desc_handle = descriptor.handle();
    GatoHandle char_handle = d->descriptor_to_characteristic.value(desc_handle);

    if (!char_handle) {
        qWarning() << "Unknown descriptor for this peripheral";
        return;
    }

    GatoHandle service_handle = d->characteristic_to_service.value(char_handle);
    Q_ASSERT(service_handle);

    GatoService &our_service = d->services[service_handle];
    Q_ASSERT(our_service.containsCharacteristic(char_handle));

    if (state() == StateConnected) {
        d->att->requestWrite(descriptor.handle(), data,
                             d, SLOT(handleDescriptorWrite(uint,bool)));
    } else {
        qWarning() << "Not connected";
    }
}

void GatoPeripheral::setNotification(const GatoCharacteristic &characteristic, bool enabled)
{
    Q_D(GatoPeripheral);

    GatoHandle char_handle = characteristic.startHandle();
    GatoHandle service_handle = d->characteristic_to_service.value(char_handle);

    if (!service_handle) {
        qWarning() << "Unknown characteristic for this peripheral";
        return;
    }

    GatoService &our_service = d->services[service_handle];
    Q_ASSERT(our_service.containsCharacteristic(char_handle));
    GatoCharacteristic our_char = our_service.getCharacteristic(char_handle);

    if (!(our_char.properties() & GatoCharacteristic::PropertyNotify)) {
        qWarning() << "Characteristic does not support notifications";
        return;
    }

    if (state() != StateConnected) {
        qWarning() << "Not connected";
        return;
    }

    const GatoUUID uuid(GatoUUID::GattClientCharacteristicConfiguration);
    if (our_char.containsDescriptor(uuid)) {
        GatoDescriptor desc = our_char.getDescriptor(uuid);
        d->pending_set_notify.remove(char_handle);
        writeValue(characteristic, d->genClientCharConfiguration(true, false));
    } else {
        d->pending_set_notify[char_handle] = enabled;
        discoverDescriptors(our_char); // May need to find appropiate descriptor
    }
}

GatoPeripheralPrivate::GatoPeripheralPrivate(GatoPeripheral *parent)
    : QObject(parent), q_ptr(parent),
      complete_name(false), complete_services(false)
{
}

GatoPeripheralPrivate::~GatoPeripheralPrivate()
{
    delete att;
}

void GatoPeripheralPrivate::parseEIRFlags(quint8 data[], int len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    // Nothing to do for now.
}

void GatoPeripheralPrivate::parseEIRUUIDs(int size, bool complete, quint8 data[], int len)
{
    Q_UNUSED(complete);

    if (size != 16/8 && size != 32/8 && size != 128/8) {
        qWarning() << "Unhandled UUID size: " << size;
        return;
    }

    for (int pos = 0; pos < len; pos += size) {
        char *ptr = reinterpret_cast<char*>(&data[pos]);
        QByteArray ba = QByteArray::fromRawData(ptr, size/8);

        service_uuids.insert(bytearray_to_gatouuid(ba));
    }
}

void GatoPeripheralPrivate::parseName(bool complete, quint8 data[], int len)
{
    Q_Q(GatoPeripheral);
    if (complete || !complete_name) {
        name = QString::fromUtf8(reinterpret_cast<char*>(data), len);
        complete_name = complete;
        emit q->nameChanged();
    }
}

GatoCharacteristic GatoPeripheralPrivate::parseCharacteristicValue(const QByteArray &ba)
{
    GatoCharacteristic characteristic;
    const char *data = ba.constData();

    quint8 properties = data[0];
    characteristic.setProperties(GatoCharacteristic::Properties(properties));

    GatoHandle handle = read_le<quint16>(&data[1]);
    characteristic.setValueHandle(handle);

    GatoUUID uuid = bytearray_to_gatouuid(ba.mid(3));
    characteristic.setUuid(uuid);

    return characteristic;
}

QByteArray GatoPeripheralPrivate::genClientCharConfiguration(bool notification, bool indication)
{
    QByteArray ba;
    ba.resize(sizeof(quint16));

    quint16 val = 0;
    if (notification)
        val |= 0x1;
    if (indication)
        val |= 0x2;

    write_le<quint16>(val, ba.data());

    return ba;
}

void GatoPeripheralPrivate::clearServices()
{
    characteristic_to_service.clear();
    value_to_characteristic.clear();
    descriptor_to_characteristic.clear();
    services.clear();
}

void GatoPeripheralPrivate::clearServiceCharacteristics(GatoService *service)
{
    QList<GatoCharacteristic> chars = service->characteristics();
    QList<GatoCharacteristic>::iterator it;
    for (it = chars.begin(); it != chars.end(); ++it) {
        clearCharacteristicDescriptors(&*it);
        characteristic_to_service.remove(it->startHandle());
        value_to_characteristic.remove(it->valueHandle());
    }
    service->clearCharacteristics();
}

void GatoPeripheralPrivate::clearCharacteristicDescriptors(GatoCharacteristic *characteristic)
{
    QList<GatoDescriptor> descs = characteristic->descriptors();
    foreach (const GatoDescriptor& d, descs) {
        descriptor_to_characteristic.remove(d.handle());
    }
    characteristic->clearDescriptors();
}

void GatoPeripheralPrivate::finishSetNotifyOperations(const GatoCharacteristic &characteristic)
{
    Q_Q(GatoPeripheral);

    GatoHandle handle = characteristic.startHandle();

    if (pending_set_notify.contains(handle)) {
        const GatoUUID uuid(GatoUUID::GattClientCharacteristicConfiguration);
        bool notify = pending_set_notify.value(handle);

        foreach (const GatoDescriptor &descriptor, characteristic.descriptors()) {
            if (descriptor.uuid() == uuid) {
                q->writeValue(descriptor, genClientCharConfiguration(notify, false));
            }
        }

        pending_set_notify.remove(handle);
    }
}

void GatoPeripheralPrivate::handleAttConnected()
{
    Q_Q(GatoPeripheral);

    emit q->connected();
}

void GatoPeripheralPrivate::handleAttDisconnected()
{
    Q_Q(GatoPeripheral);

    // Forget about all pending requests
    pending_primary_reqs.clear();
    pending_characteristic_reqs.clear();
    pending_characteristic_read_reqs.clear();
    pending_descriptor_reqs.clear();
    pending_descriptor_read_reqs.clear();

    emit q->disconnected();
}

void GatoPeripheralPrivate::handleAttAttributeUpdated(GatoHandle handle, const QByteArray &value, bool confirmed)
{
    Q_Q(GatoPeripheral);
    Q_UNUSED(confirmed);

    // Let's see if this is a handle we know about.
    if (value_to_characteristic.contains(handle)) {
        // Ok, it's a characteristic value.
        GatoHandle char_handle = value_to_characteristic.value(handle);
        GatoHandle service_handle = characteristic_to_service.value(char_handle);
        if (!service_handle) {
            qWarning() << "Got a notification for a characteristic I don't know about";
            return;
        }

        GatoService &service = services[service_handle];
        GatoCharacteristic characteristic = service.getCharacteristic(char_handle);

        emit q->valueUpdated(characteristic, value);
    }
}

void GatoPeripheralPrivate::handlePrimary(uint req, const QList<GatoAttClient::AttributeGroupData> &list)
{
    Q_Q(GatoPeripheral);
    Q_UNUSED(req);

    if (list.isEmpty()) {
        complete_services = true;
        emit q->servicesDiscovered();
    } else {
        GatoHandle last_handle = 0;

        foreach (const GatoAttClient::AttributeGroupData &data, list) {
            GatoUUID uuid = bytearray_to_gatouuid(data.value);
            GatoService service;

            service.setUuid(uuid);
            service.setStartHandle(data.start);
            service.setEndHandle(data.end);

            services.insert(data.start, service);
            service_uuids.insert(uuid);

            last_handle = data.end;
        }

        // Fetch following attributes
        att->requestReadByGroupType(last_handle + 1, 0xFFFF, GatoUUID::GattPrimaryService,
                                    this, SLOT(handlePrimary(uint,QList<GatoAttClient::AttributeGroupData>)));
    }
}

void GatoPeripheralPrivate::handlePrimaryForService(uint req, const QList<GatoAttClient::HandleInformation> &list)
{
    Q_Q(GatoPeripheral);

    GatoUUID uuid = pending_primary_reqs.value(req, GatoUUID());
    if (uuid.isNull()) {
        qDebug() << "Got primary for service response for a request I did not make";
        return;
    }
    pending_primary_reqs.remove(req);

    if (list.isEmpty()) {
        if (pending_primary_reqs.isEmpty()) {
            emit q->servicesDiscovered();
        }
    } else {
        GatoHandle last_handle = 0;

        foreach (const GatoAttClient::HandleInformation &data, list) {
            GatoService service;

            service.setUuid(uuid);
            service.setStartHandle(data.start);
            service.setEndHandle(data.end);

            services.insert(data.start, service);
            service_uuids.insert(uuid);

            last_handle = data.end;
        }

        // Fetch following attributes
        QByteArray value = gatouuid_to_bytearray(uuid, true, false);
        uint req = att->requestFindByTypeValue(last_handle + 1, 0xFFFF, GatoUUID::GattPrimaryService, value,
                                               this, SLOT(handlePrimaryForService(uint,QList<GatoAttClient::HandleInformation>)));
        pending_primary_reqs.insert(req, uuid);
    }
}

void GatoPeripheralPrivate::handleCharacteristic(uint req, const QList<GatoAttClient::AttributeData> &list)
{
    Q_Q(GatoPeripheral);

    GatoHandle service_start = pending_characteristic_reqs.value(req, 0);
    if (!service_start) {
        qDebug() << "Got characteristics for a request I did not make";
        return;
    }
    pending_characteristic_reqs.remove(req);

    Q_ASSERT(services.contains(service_start));
    GatoService &service = services[service_start];
    Q_ASSERT(service.startHandle() == service_start);

    if (list.isEmpty()) {
        emit q->characteristicsDiscovered(service);
    } else {
        GatoHandle last_handle = 0;

        // If we are continuing a characteristic list, this means the
        // last service we discovered in the previous iteration was not
        // the last one, so we have to reduce its endHandle!
        QList<GatoCharacteristic> cur_chars = service.characteristics();
        if (!cur_chars.isEmpty()) {
            GatoCharacteristic &last = cur_chars.back();
            last.setEndHandle(list.front().handle - 1);
            service.addCharacteristic(last);
        }

        for (int i = 0; i < list.size(); i++) {
            const GatoAttClient::AttributeData &data = list.at(i);
            GatoCharacteristic characteristic = parseCharacteristicValue(data.value);

            characteristic.setStartHandle(data.handle);
            if (i + 1 < list.size()) {
                characteristic.setEndHandle(list.at(i + 1).handle - 1);
            } else {
                characteristic.setEndHandle(service.endHandle());
            }

            service.addCharacteristic(characteristic);
            characteristic_to_service.insert(data.handle, service_start);
            value_to_characteristic.insert(characteristic.valueHandle(), data.handle);

            last_handle = data.handle;
        }

        if (last_handle >= service.endHandle()) {
            // Already finished, no need to send another request
            emit q->characteristicsDiscovered(service);
            return;
        }

        // Fetch following attributes
        uint req = att->requestReadByType(last_handle + 1, service.endHandle(), GatoUUID::GattCharacteristic,
                                          this, SLOT(handleCharacteristic(uint,QList<GatoAttClient::AttributeData>)));
        pending_characteristic_reqs.insert(req, service.startHandle());
    }
}

void GatoPeripheralPrivate::handleDescriptors(uint req, const QList<GatoAttClient::InformationData> &list)
{
    Q_Q(GatoPeripheral);

    GatoHandle char_handle = pending_descriptor_reqs.value(req);
    if (!char_handle) {
        qDebug() << "Got descriptor for a request I did not make";
        return;
    }
    pending_descriptor_reqs.remove(req);
    GatoHandle service_handle = characteristic_to_service.value(char_handle);
    if (!service_handle) {
        qWarning() << "Unknown characteristic during descriptor discovery: " << char_handle;
        return;
    }

    Q_ASSERT(services.contains(service_handle));
    GatoService &service = services[service_handle];
    Q_ASSERT(service.startHandle() == service_handle);

    Q_ASSERT(service.containsCharacteristic(char_handle));
    GatoCharacteristic characteristic = service.getCharacteristic(char_handle);

    if (list.isEmpty()) {
        finishSetNotifyOperations(characteristic);
        emit q->descriptorsDiscovered(characteristic);
    } else {
        GatoHandle last_handle = 0;

        foreach (const GatoAttClient::InformationData &data, list) {
            // Skip the value attribute itself.
            if (data.handle == characteristic.valueHandle()) continue;

            GatoDescriptor descriptor;

            descriptor.setHandle(data.handle);
            descriptor.setUuid(data.uuid);

            characteristic.addDescriptor(descriptor);

            service.addCharacteristic(characteristic);
            descriptor_to_characteristic.insert(data.handle, char_handle);

            last_handle = data.handle;
        }

        service.addCharacteristic(characteristic);

        if (last_handle >= characteristic.endHandle()) {
            // Already finished, no need to send another request
            finishSetNotifyOperations(characteristic);
            emit q->descriptorsDiscovered(characteristic);
            return;
        }

        // Fetch following attributes
        uint req = att->requestFindInformation(last_handle + 1, characteristic.endHandle(),
                                               this, SLOT(handleDescriptors(uint,QList<GatoAttClient::InformationData>)));
        pending_descriptor_reqs.insert(req, char_handle);

    }
}

void GatoPeripheralPrivate::handleCharacteristicRead(uint req, const QByteArray &value)
{
    Q_Q(GatoPeripheral);

    GatoHandle char_handle = pending_characteristic_read_reqs.value(req);
    if (!char_handle) {
        qDebug() << "Got characteristics for a request I did not make";
        return;
    }
    pending_characteristic_read_reqs.remove(req);
    GatoHandle service_handle = characteristic_to_service.value(char_handle);
    if (!service_handle) {
        qWarning() << "Unknown characteristic during read: " << char_handle;
        return;
    }

    Q_ASSERT(services.contains(service_handle));
    GatoService &service = services[service_handle];
    Q_ASSERT(service.startHandle() == service_handle);

    Q_ASSERT(service.containsCharacteristic(char_handle));
    GatoCharacteristic characteristic = service.getCharacteristic(char_handle);

    emit q->valueUpdated(characteristic, value);
}

void GatoPeripheralPrivate::handleDescriptorRead(uint req, const QByteArray &value)
{
    Q_Q(GatoPeripheral);

    GatoHandle desc_handle = pending_descriptor_read_reqs.value(req);
    if (!desc_handle) {
        qDebug() << "Got characteristics for a request I did not make";
        return;
    }
    pending_descriptor_read_reqs.remove(req);
    GatoHandle char_handle = descriptor_to_characteristic.value(desc_handle);
    if (!char_handle) {
        qWarning() << "Unknown characteristic during read: " << char_handle;
        return;
    }
    GatoHandle service_handle = characteristic_to_service.value(char_handle);
    if (!service_handle) {
        qWarning() << "Unknown characteristic during read: " << char_handle;
        return;
    }

    Q_ASSERT(services.contains(service_handle));
    GatoService &service = services[service_handle];
    Q_ASSERT(service.startHandle() == service_handle);

    Q_ASSERT(service.containsCharacteristic(char_handle));
    GatoCharacteristic characteristic = service.getCharacteristic(char_handle);

    Q_ASSERT(characteristic.containsDescriptor(desc_handle));
    GatoDescriptor descriptor = characteristic.getDescriptor(desc_handle);

    emit q->descriptorValueUpdated(descriptor, value);
}

void GatoPeripheralPrivate::handleCharacteristicWrite(uint req, bool ok)
{
    Q_UNUSED(req);
    if (!ok) {
        qWarning() << "Failed to write some characteristic";
    }
}

void GatoPeripheralPrivate::handleDescriptorWrite(uint req, bool ok)
{
    Q_UNUSED(req);
    if (!ok) {
        qWarning() << "Failed to write some characteristic";
    }
}

QT_END_NAMESPACE
