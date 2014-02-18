/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited all rights reserved
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

#include <QtCore/QLoggingCategory>
#include "qlowenergycharacteristicinfo_p.h"
#include <btapi/btdevice.h>
#include <btapi/btgatt.h>
#include <btapi/btle.h>
#include <errno.h>
#include "qlowenergyserviceinfo.h"
#include "qlowenergyserviceinfo_p.h"
#include "qlowenergydescriptorinfo_p.h"
#include "qnx/ppshelpers_p.h"


QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_QNX)

int hexValue(char inChar)
{
    if (isxdigit(inChar)) {
        if (isdigit(inChar)) {
            return (inChar - '0');
        } else {
            return (toupper(inChar) - 'A' + 10);
        }
    } else {
        return -1;
    }
}

int stringToBuffer(const QString &stringData, uint8_t *buffer, int bufferLength)
{
    int consumed = 0;
    for (int i = 0; i < bufferLength; i++) {
        const int hex = hexValue(stringData.at(i).toLatin1());
        if (hex >= 0) {
            if ((consumed % 2) == 0) {
                buffer[(consumed / 2)] = hex << 4;
            } else {
                buffer[(consumed / 2)] |= hex;
            }

            consumed++;
        }
    }

    // Round up the number of bytes we consumed to a multiple of 2.
    if ((consumed % 2) != 0)
        ++consumed;

    return consumed;
}

QLowEnergyCharacteristicInfoPrivate::QLowEnergyCharacteristicInfoPrivate():
    name(QString()), value(QByteArray()), permission(0), instance(-1), handle(QStringLiteral("0x0000")), properties(QVariantMap()), errorString("")
{

}

QLowEnergyCharacteristicInfoPrivate::~QLowEnergyCharacteristicInfoPrivate()
{

}

void QLowEnergyCharacteristicInfoPrivate::serviceNotification(int instance, short unsigned int handle, const unsigned char*val, short unsigned int len, void *userData)
{
    if (val == 0)
        return;
    QPointer<QLowEnergyServiceInfoPrivate> *ClassPointer = static_cast<QPointer<QLowEnergyServiceInfoPrivate> *>(userData);
    QLowEnergyServiceInfoPrivate *p = ClassPointer->data();
    qCDebug(QT_BT_QNX) << "---------------------------------------------------";
    qCDebug(QT_BT_QNX) << "[Notification] received (service uuid, handle, value, instance):" << p->uuid << handle << val << instance;
    qCDebug(QT_BT_QNX) << "---------------------------------------------------";
    //Check if the notification from wanted characteristic
    bool current = false;
    QLowEnergyCharacteristicInfo chars;
    for (int i = 0; i < p->characteristicList.size(); i++) {
        QString charHandle;
        charHandle.setNum(handle);
        charHandle = charHandle;
        if (charHandle == ((QLowEnergyCharacteristicInfo)p->characteristicList.at(i)).handle() ) {
            chars = QLowEnergyCharacteristicInfo((QLowEnergyCharacteristicInfo)p->characteristicList.at(i));
            QByteArray receivedValue;

            for (int j = 0; j < len; j++) {
                QString hexadecimal;
                hexadecimal.setNum(val[j], 16);
                receivedValue.append(hexadecimal.toLatin1());
            }

            p->characteristicList.at(i).d_ptr->setValue(receivedValue);
            current = true;
        }
    }

    if (!current)
        qCDebug(QT_BT_QNX) << "Notificiation received and does not belong to this characteristic.";
}

bool QLowEnergyCharacteristicInfoPrivate::enableNotification()
{
    if (instance == -1) {
        qCDebug(QT_BT_QNX) << " GATT service not connected ";
        errorString = QStringLiteral("Service is not connected");
        emit error(uuid);
        return false;
    }
    if ( (permission & QLowEnergyCharacteristicInfo::Notify) == 0) {
        qCDebug(QT_BT_QNX) << "Notification changes not allowed";
        errorString = QStringLiteral("This characteristic does not support notifications.");
        emit error(uuid);
        return false;
    }

    int rc = bt_gatt_enable_notify(instance, &characteristic, 1);
    if (rc != 0) {
        qCDebug(QT_BT_QNX) << "bt_gatt_enable_notify errno=" << errno << strerror(errno);
        errorString = QString::fromLatin1(strerror(errno));
        emit error(uuid);
        return false;
    } else {
        qCDebug(QT_BT_QNX) << "bt_gatt_enable_notify was presumably OK";
        return true;
    }
}

void QLowEnergyCharacteristicInfoPrivate::setValue(const QByteArray &wantedValue)
{
    if (permission & QLowEnergyCharacteristicInfo::Write) {
        const int characteristicLen = wantedValue.size();
        uint8_t *characteristicBuffer = (uint8_t *)alloca(characteristicLen / 2 + 1);
        if (!characteristicBuffer) {
            qCDebug(QT_BT_QNX) << "GATT characteristic: Not enough memory";
            errorString = QStringLiteral("Not enough memory.");
            emit error(uuid);
            bt_gatt_disconnect_instance(instance);
            return;
        }

        const int consumed = stringToBuffer(QString::fromLocal8Bit(wantedValue), characteristicBuffer, characteristicLen);

        if (consumed > 0) {
            int byteCount;
            byteCount = bt_gatt_write_value(instance, handle.toUShort(), 0, characteristicBuffer, (consumed / 2));

            if (byteCount < 0) {
                qCDebug(QT_BT_QNX) << "Unable to write characteristic value: " << errno << strerror(errno);
                errorString = QStringLiteral("Unable to write characteristic value: ") + QString::fromLatin1(strerror(errno));
                emit error(uuid);
            }
        }
    }
    else {
        errorString = QStringLiteral("Characteristic does not allow write operations. The wanted value was not written to the device.");
        emit error(uuid);
    }
    value = wantedValue;
    properties[QStringLiteral("value")] = value;
    emit notifyValue(uuid);

}

void QLowEnergyCharacteristicInfoPrivate::disableNotification()
{
    int rc = bt_gatt_enable_notify(instance, &characteristic, 0);
    if (rc != 0)
        qCDebug(QT_BT_QNX) << "bt_gatt_enable_notify errno=" << errno << strerror(errno);
    else
        qCDebug(QT_BT_QNX) << "bt_gatt_enable_notify was presumably OK";

}

void QLowEnergyCharacteristicInfoPrivate::readDescriptors()
{
    descriptorsList.clear();
    int count = bt_gatt_descriptors_count(instance, &characteristic);

    if (count == -1) {
        qWarning() << "GATT descriptors count failed:" << errno << "(" << strerror(errno) << ")";
        bt_gatt_disconnect_instance(instance);
        return;
    }

    bt_gatt_descriptor_t *descriptorList = 0;
    if (count > 0) {
        descriptorList = (bt_gatt_descriptor_t*)alloca(count * sizeof(bt_gatt_descriptor_t));
        if (!descriptorList) {
            qCDebug(QT_BT_QNX) <<"GATT descriptors: Not enough memory";
            bt_gatt_disconnect_instance(instance);
            return;
        }

        /* BEGIN WORKAROUND - Temporary fix to address race condition */
        int number = 0;
        do {
            number = bt_gatt_descriptors(instance, &characteristic, descriptorList, count);
        } while ((number == -1) && (errno == EBUSY));

        count = number;
        /* END WORKAROUND */
    }

    if (count == -1) {
        qCDebug(QT_BT_QNX) << "GATT descriptors failed: %1 (%2)" << errno << strerror(errno);
        bt_gatt_disconnect_instance(instance);
        return;
    }

    characteristicMtu = bt_gatt_get_mtu(instance);

    uint8_t *descriptorBuffer = (uint8_t *)alloca(characteristicMtu);
    if (!descriptorBuffer) {
        qCDebug(QT_BT_QNX) <<"GATT descriptors: Not enough memory";
        bt_gatt_disconnect_instance(instance);
        return;
    }

    for (int i = 0; i < count; i++) {
        QVariantMap map;

        map[QStringLiteral("uuid")] = QString::fromLatin1(descriptorList[i].uuid);

        map[QStringLiteral("handle")] = descriptorList[i].handle;
        QString descHanlde;
        descHanlde.setNum(descriptorList[i].handle);
        QString descriptorUuid(descriptorList[i].uuid);
        QBluetoothUuid descUuid(descriptorUuid);
        QLowEnergyDescriptorInfo descriptor(descUuid, descHanlde);

        uint8_t more = 1;
        int byteCount;
        for (int offset = 0; more; offset += byteCount) {
            byteCount = bt_gatt_read_value(instance, descriptorList[i].handle, offset, descriptorBuffer, characteristicMtu, &more);
            if (byteCount < 0) {
                qCDebug(QT_BT_QNX) << "Unable to read descriptor value:"<< errno<< strerror(errno);
                break;
            }
            descriptor.d_ptr->m_value = QByteArray();
            for (int j = 0; j < byteCount; j++) {
                QString hexadecimal;
                hexadecimal.setNum(descriptorBuffer[j], 16);
                descriptor.d_ptr->m_value.append(hexadecimal.toLatin1());
            }

        }

        map[QStringLiteral("value")] = descriptor.d_ptr->m_value;
        descriptor.d_ptr->m_properties = map;
        descriptorsList.append(descriptor);

    }
}

void QLowEnergyCharacteristicInfoPrivate::readValue()
{
    if ((permission & QLowEnergyCharacteristicInfo::Read) == 0) {
        qCDebug(QT_BT_QNX) << "GATT characteristic: Read not permitted";
        return;
    }

    uint8_t *characteristicBuffer = (uint8_t *)alloca(characteristicMtu);
    if (!characteristicBuffer) {
        qCDebug(QT_BT_QNX) << "GATT characteristic: Not enough memory";
        bt_gatt_disconnect_instance(instance);
        return;
    }

    QString descriptorString;

    int byteCount = 0;
    uint8_t more = 1;
    for (int offset = 0; more; offset += byteCount) {
        byteCount = bt_gatt_read_value(instance, handle.toUShort(), offset, characteristicBuffer, characteristicMtu, &more);
        if (byteCount < 0) {
            qCDebug(QT_BT_QNX) << "Unable to read characteristic value: " << errno << strerror(errno);
            break;
        }
        value = QByteArray();
        for (int j = 0; j < byteCount; j++) {
            QString hexadecimal;
            hexadecimal.setNum(characteristicBuffer[j], 16);
            value.append(hexadecimal.toLatin1());
        }
        properties["value"] = value;
    }
}

bool QLowEnergyCharacteristicInfoPrivate::valid()
{
    if (instance == -1)
        return false;
    return true;
}

QT_END_NAMESPACE
