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
#include "qlowenergycontroller_p.h"
#include "qlowenergyserviceinfo_p.h"
#include "qlowenergycharacteristicinfo.h"
#include "qlowenergycharacteristicinfo_p.h"
#include "qlowenergydescriptorinfo.h"
#include "qlowenergydescriptorinfo_p.h"
#include "qlowenergyprocess_p.h"
#include <btapi/btdevice.h>
#include <btapi/btgatt.h>
#include <btapi/btspp.h>
#include <btapi/btle.h>
#include <errno.h>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_QNX)

int hexValue(QChar inChar)
{
    if (inChar.isDigit())
        return (inChar.unicode() - '0');
    else
        return (inChar.toUpper().unicode() - 'A' + 10);
    return -1;
}

int stringToBuffer(const QString &stringData, quint8 *buffer, int bufferLength)
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

void QLowEnergyControllerPrivate::serviceConnected(const char *bdaddr, const char *service, int instance,
                                                   int err, short unsigned int connInt, short unsigned int latency,
                                                   short unsigned int superTimeout, void *userData)
{
    Q_UNUSED(latency);
    Q_UNUSED(connInt);
    Q_UNUSED(superTimeout);
    QSharedPointer<QLowEnergyControllerPrivate> *classPointer =
            static_cast<QSharedPointer<QLowEnergyControllerPrivate> *>(userData);
    QLowEnergyControllerPrivate *p = classPointer->data();

    qCDebug(QT_BT_QNX) << "---------------------------------------------------";
    qCDebug(QT_BT_QNX) << "[SERVICE: Connected] (instance):" << instance;
    qCDebug(QT_BT_QNX) << "[SERVICE: Connected] Device address: " << bdaddr;
    qCDebug(QT_BT_QNX) << "[SERVICE: Connected] Device service: " << service;
    qCDebug(QT_BT_QNX) << "[SERVICE: Connected] Possible error: " << err;

    QString lowEnergyUuid(service);
    QBluetoothUuid leUuid;

    //In case of custom UUIDs (e.g. Texas Instruments SenstorTag LE Device)
    if (lowEnergyUuid.length() > 6) {
        lowEnergyUuid.remove(0,2);
        leUuid = QBluetoothUuid(lowEnergyUuid);
    } else { // Official UUIDs are presented in 6 characters (for instance 0x180A)
        leUuid = QBluetoothUuid(lowEnergyUuid.toUShort(0,0));
    }
    qCDebug(QT_BT_QNX) << leUuid;

    int index = -1;
    for (int i = 0; i < p->m_leServices.size(); i++) {
        if (p->m_leServices.at(i).serviceUuid() == leUuid) {
            index = i;
            break;
        }
    }

    if (err != 0) {
        qCDebug(QT_BT_QNX) << "An error occurred in service connected callback: " << qt_error_string(err);
        p->errorString = qt_error_string(err);
        p->q_ptr->error(p->m_leServices.at(index));
    }
    if (index != -1) {
        p->m_leServices.at(index).d_ptr->characteristicList.clear();
        bt_gatt_characteristic_t* data;
        data = (bt_gatt_characteristic_t*) malloc(sizeof(bt_gatt_characteristic_t));
        if (!data) {
            qCDebug(QT_BT_QNX) << "[SERVICE: Connected] GATT characteristics: Not enough memory";
            bt_gatt_disconnect_instance(instance);
            p->errorString = QStringLiteral("GATT characteristics: Not enough memory");
            p->q_ptr->error(p->m_leServices.at(index));
            return;
        }

        int num_characteristics = bt_gatt_characteristics_count(instance);

        if (num_characteristics > -1) {
            qCDebug(QT_BT_QNX) << "Characteristics number: "<< num_characteristics;
            bt_gatt_characteristic_t *allCharacteristicList;

            allCharacteristicList = (bt_gatt_characteristic_t*) malloc(num_characteristics * sizeof(bt_gatt_characteristic_t));
            if (!allCharacteristicList) {
                qCDebug(QT_BT_QNX) <<" GATT characteristics: Not enough memory";
                bt_gatt_disconnect_instance(instance);
                p->errorString = QStringLiteral("GATT characteristics: Not enough memory");
                p->q_ptr->error(p->m_leServices.at(index));
                return;
            }

            /* BEGIN WORKAROUND - Temporary fix to address race condition */
            int number = 0;
            do {
                number = bt_gatt_characteristics(instance, allCharacteristicList, num_characteristics);
            } while ((number == -1) && (errno == EBUSY));
            //Using sub to subscribe notification callback only once
            bool sub = false;
            int characteristicListSize = number;

            for (int i = 0; i < characteristicListSize; i++) {
                qCDebug(QT_BT_QNX) << "Characteristic: uuid,handle,value_handle, properties:"
                                   << allCharacteristicList[i].uuid << "," << allCharacteristicList[i].handle
                                   << "," << allCharacteristicList[i].value_handle
                                   << ", " << allCharacteristicList[i].properties;
                QString charUuid = QString::fromLatin1(allCharacteristicList[i].uuid);
                QString handleUuid;
                handleUuid.setNum(allCharacteristicList[i].value_handle);
                QBluetoothUuid characteristicUuid;
                if (!charUuid.toUShort(0,0)) {
                    charUuid = charUuid.remove(0,2);
                    characteristicUuid = QBluetoothUuid(charUuid);
                } else {
                    characteristicUuid = QBluetoothUuid(charUuid.toUShort(0,0));
                }
                QVariantMap map;
                QLowEnergyCharacteristicInfo characteristicInfo(characteristicUuid);
                characteristicInfo.d_ptr->handle = handleUuid;
                characteristicInfo.d_ptr->instance = instance;
                characteristicInfo.d_ptr->characteristic = allCharacteristicList[i];
                characteristicInfo.d_ptr->permission = allCharacteristicList[i].properties;
                map[QStringLiteral("uuid")] = characteristicUuid.toString();
                map[QStringLiteral("handle")] = handleUuid;
                map[QStringLiteral("permission")] = characteristicInfo.d_ptr->permission;
                characteristicInfo.d_ptr->properties = map;
                p->readDescriptors(characteristicInfo);
                p->readValue(characteristicInfo);
                //Subscribe only once since it is static function
                if (!sub) {
                    int rc = bt_gatt_reg_notifications(instance, &(p->serviceNotification));
                    if (!rc) {
                        qCDebug(QT_BT_QNX) << "[SERVICE: Connected] bt_gatt_reg_notifications failed." << errno << qt_error_string(errno);
                        p->errorString = qt_error_string(errno);
                        p->q_ptr->error(p->m_leServices.at(index));
                    } else {
                        qCDebug(QT_BT_QNX) << "[SERVICE: Connected] bt_gatt_reg_notifications was presumably OK";
                    }
                    sub = true;
                }
                p->m_leServices.at(index).d_ptr->characteristicList.append(characteristicInfo);

            }

            if (!allCharacteristicList) {
                free(allCharacteristicList);
                allCharacteristicList = 0;
            }

            /* END WORKAROUND */
        }

        p->m_leServices.at(index).d_ptr->connected = true;

        qCDebug(QT_BT_QNX) << p;
        emit p->q_ptr->connected(p->m_leServices.at(index));
        qCDebug(QT_BT_QNX) << "---------------------------------------------------------------------------------";
    } else {
        qCDebug(QT_BT_QNX) << "Unregistered service connected";
    }
}

void QLowEnergyControllerPrivate::serviceNotification(int instance, short unsigned int handle,
                                                      const unsigned char*val, short unsigned int len, void *userData)
{
    if (!val)
        return;
    QSharedPointer<QLowEnergyControllerPrivate> *ClassPointer
            = static_cast<QSharedPointer<QLowEnergyControllerPrivate> *>(userData);
    QLowEnergyControllerPrivate *p = ClassPointer->data();

    qCDebug(QT_BT_QNX) << "---------------------------------------------------";
    qCDebug(QT_BT_QNX) << "[Notification] received (handle, value, instance):" << handle << val << instance;
    qCDebug(QT_BT_QNX) << "---------------------------------------------------";

    //Check if the notification from wanted characteristic
    bool current = false;
    QLowEnergyCharacteristicInfo chars;
    for (int i = 0; i < p->m_leServices.size(); i++) {
        for (int j = 0; j < p->m_leServices.at(i).characteristics().size(); j++) {

            QString charHandle;
            charHandle.setNum(handle);
            charHandle = charHandle;
            if (charHandle == p->m_leServices.at(i).d_ptr->characteristicList.at(j).handle() ) {
                chars = QLowEnergyCharacteristicInfo(p->m_leServices.at(i).d_ptr->characteristicList.at(j));
                QByteArray receivedValue;

                for (int k = 0; k < len; k++) {
                    QByteArray hexadecimal;
                    hexadecimal.append(val[k]);
                    receivedValue.append(hexadecimal.toHex());
                }

                p->m_leServices.at(i).d_ptr->characteristicList.at(j).d_ptr->notification = true;
                p->m_leServices.at(i).d_ptr->characteristicList.at(j).d_ptr->value = receivedValue;
                p->q_ptr->valueChanged(p->m_leServices.at(i).d_ptr->characteristicList.at(j));
                current = true;
            }
        }
    }

    if (!current)
        qCDebug(QT_BT_QNX) << "Notificiation received and does not belong to this characteristic.";
}

void QLowEnergyControllerPrivate::serviceUpdate(const char *bdaddr, int instance,
                                                short unsigned int connInt, short unsigned int latency,
                                                short unsigned int superTimeout, void *userData)
{
    Q_UNUSED(latency);
    Q_UNUSED(connInt);
    Q_UNUSED(superTimeout);
    Q_UNUSED(userData);
    qCDebug(QT_BT_QNX) << "---------------------------------------------------";
    qCDebug(QT_BT_QNX) << "[SERVICE: Update] (instance):" << instance;
    qCDebug(QT_BT_QNX) << "[SERVICE: Update] Device address: " << bdaddr;
    qCDebug(QT_BT_QNX) << "---------------------------------------------------";
}

void QLowEnergyControllerPrivate::serviceDisconnected(const char *bdaddr, const char *service,
                                                      int instance, int reason, void *userData)
{
    QSharedPointer<QLowEnergyControllerPrivate> *classPointer = static_cast<QSharedPointer<QLowEnergyControllerPrivate> *>(userData);
    QLowEnergyControllerPrivate *p = classPointer->data();
    QString lowEnergyUuid(service);
    qCDebug(QT_BT_QNX) << "LE Service: " << lowEnergyUuid << service;
    QBluetoothUuid leUuid;

    //In case of custom UUIDs (e.g. Texas Instruments SenstorTag LE Device)
    if ( lowEnergyUuid.length() > 4 ) {
        leUuid = QBluetoothUuid(lowEnergyUuid);
    }
    else {// Official UUIDs are presented in 4 characters (for instance 180A)
        lowEnergyUuid = QStringLiteral("0x") + lowEnergyUuid;
        leUuid = QBluetoothUuid(lowEnergyUuid.toUShort(0,0));
    }
    QLowEnergyServiceInfo leService(leUuid);
    emit p->q_ptr->connected(leService);

    qCDebug(QT_BT_QNX) << "---------------------------------------------------";
    qCDebug(QT_BT_QNX) << "[SERVICE: Disconnect] (service, instance, reason):" << service << instance << reason;
    qCDebug(QT_BT_QNX) << "[SERVICE: Disconnect] Device address: " << bdaddr;
    qCDebug(QT_BT_QNX) << "---------------------------------------------------";
    delete p;
    delete classPointer;
}

QLowEnergyControllerPrivate::QLowEnergyControllerPrivate()
{
    qRegisterMetaType<QLowEnergyServiceInfo>("QLowEnergyServiceInfo");
    qRegisterMetaType<QLowEnergyCharacteristicInfo>("QLowEnergyCharacteristicInfo");
}

QLowEnergyControllerPrivate::~QLowEnergyControllerPrivate()
{

}

void QLowEnergyControllerPrivate::connectService(const QLowEnergyServiceInfo &service)
{
    if (!service.isValid()) {
        errorString = QStringLiteral("Service not valid.");
        emit q_ptr->error(service);
        return;
    }

    bool add = true;
    for (int i = 0; i < m_leServices.size(); i++) {
        if (m_leServices.at(i).serviceUuid() == service.serviceUuid()) {
            if (m_leServices.at(i).isConnected()) {
                errorString = QStringLiteral("Service already connected");
                emit q_ptr->error(m_leServices.at(i));
            }
            else {
                m_leServices.replace(i, service);
                add = false;
                break;
            }
        }
    }
    if (add)
        m_leServices.append(service);

    bt_gatt_callbacks_t gatt_callbacks = {&(this->serviceConnected), this->serviceDisconnected, this->serviceUpdate};

    errno = 0;
    process = process->instance();
    if (!process->isConnected()) {
        qCDebug(QT_BT_QNX) << "[INIT] Init problem." << errno << qt_error_string(errno);
        errorString = QStringLiteral("Initialization of device falied. ") + qt_error_string(errno);
        emit q_ptr->error(service);
        return;
    }

    errno = 0;
    if (bt_gatt_init(&gatt_callbacks) < 0) {
        qCDebug(QT_BT_QNX) << "[INIT] GAT Init problem." << errno << qt_error_string(errno);
        errorString = QStringLiteral("Callbacks initialization failed. ") + qt_error_string(errno);
        emit q_ptr->error(service);
        return;
    }
    if (bt_le_init(0) != EOK) {
        qWarning() << "LE initialization failure " << errno;
    }

    QSharedPointer<QLowEnergyControllerPrivate> *classPointer = new QSharedPointer<QLowEnergyControllerPrivate>(this);
    process->addPointer(classPointer->data());
    QString serviceUuid = service.serviceUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
    if (service.serviceName() == QStringLiteral("Unknown Service"))
        serviceUuid = QStringLiteral("0x") + serviceUuid.toUpper();
    else
        serviceUuid = QStringLiteral("0x") + serviceUuid[4] + serviceUuid[5] + serviceUuid[6] + serviceUuid[7];
    errno = 0;

    bt_gatt_conn_parm_t conParm;
    conParm.minConn = 0x30;
    conParm.maxConn = 0x50;
    conParm.latency = 0;
    conParm.superTimeout = 50;

    if (bt_gatt_connect_service(service.device().address().toString().toLocal8Bit().constData(), serviceUuid.toLocal8Bit().constData(), 0, &conParm, classPointer) < 0) {
        qCDebug(QT_BT_QNX) << "[SERVICE] Connection to service failed." << errno << qt_error_string(errno);
        errorString = QStringLiteral("[SERVICE] Connection to service failed.") + qt_error_string(errno);
        emit q_ptr->error(service);
    }
    qCDebug(QT_BT_QNX) << "errno after connect: " << errno;
}

void QLowEnergyControllerPrivate::disconnectService(const QLowEnergyServiceInfo &leService)
{
    if (leService.isValid()){
        QString serviceUuid = leService.serviceUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
        if (leService.serviceName() == QStringLiteral("Unknown Service"))
            serviceUuid = QStringLiteral("0x") + serviceUuid.toUpper();
        else
            serviceUuid = QStringLiteral("0x") + serviceUuid[4] + serviceUuid[5] + serviceUuid[6] + serviceUuid[7];
        if (leService.isConnected()) {
            if (bt_gatt_disconnect_service(leService.device().address().toString().toLocal8Bit().constData(), serviceUuid.toLocal8Bit().constData()) < 0) {
                qCDebug(QT_BT_QNX) << "[SERVICE] Disconnect service request failed. " << errno << qt_error_string(errno);
                errorString = QStringLiteral("[SERVICE] Disconnect service request failed. ") + qt_error_string(errno);
                emit q_ptr->error(leService);
            } else {
                for (int i = 0; i < m_leServices.size(); i++) {
                    if (leService.serviceUuid() == m_leServices.at(i).serviceUuid()) {
                        m_leServices.removeAt(i);
                        break;
                    }
                }
                leService.d_ptr->connected = false;
                emit q_ptr->disconnected(leService);
                qCDebug(QT_BT_QNX) << "[SERVICE] Disconnected from service OK.";
            }
        } else {
            errorString = QStringLiteral("Service is not connected");
            q_ptr->error(leService);
        }
    } else {
        disconnectAllServices();
    }
}

void QLowEnergyControllerPrivate::disconnectAllServices()
{
    for (int i = 0; i < m_leServices.size(); i++) {
        QString serviceUuid = m_leServices.at(i).serviceUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
        if (m_leServices.at(i).serviceName() == QStringLiteral("Unknown Service"))
            serviceUuid = QStringLiteral("0x") + serviceUuid.toUpper();
        else
            serviceUuid = QStringLiteral("0x") + serviceUuid[4] + serviceUuid[5] + serviceUuid[6] + serviceUuid[7];

        if (m_leServices.at(i).isConnected()) {
            qCDebug(QT_BT_QNX) << m_leServices.at(i).device().address().toString().toLocal8Bit().constData() << serviceUuid.toLocal8Bit().constData();
            if (bt_gatt_disconnect_service( m_leServices.at(i).device().address().toString().toLocal8Bit().constData(), serviceUuid.toLocal8Bit().constData()) < 0) {
                qCDebug(QT_BT_QNX) << "[SERVICE] Disconnect service request failed. " << errno << qt_error_string(errno);
                errorString = QStringLiteral("[SERVICE] Disconnect service request failed. ") + qt_error_string(errno);
                emit q_ptr->error( m_leServices.at(i));
            } else {
                m_leServices.at(i).d_ptr->connected = false;
                emit q_ptr->disconnected(m_leServices.at(i));
                qCDebug(QT_BT_QNX) << "[SERVICE] Disconnected from service OK.";
            }
        }
    }
    m_leServices.clear();
}

bool QLowEnergyControllerPrivate::enableNotification(const QLowEnergyCharacteristicInfo &characteristic)
{
    if (characteristic.d_ptr->instance == -1) {
        qCDebug(QT_BT_QNX) << " GATT service not connected ";
        errorString = QStringLiteral("Service is not connected");
        emit q_ptr->error(characteristic);
        return false;
    }
    if (!(characteristic.d_ptr->permission & QLowEnergyCharacteristicInfo::Notify)) {
        qCDebug(QT_BT_QNX) << "Notification changes not allowed";
        errorString = QStringLiteral("This characteristic does not support notifications.");
        emit q_ptr->error(characteristic);
        return false;
    }

    int rc = bt_gatt_enable_notify(characteristic.d_ptr->instance, &characteristic.d_ptr->characteristic, 1);
    if (!rc) {
        qCDebug(QT_BT_QNX) << "bt_gatt_enable_notify errno=" << errno << qt_error_string(errno);
        errorString = qt_error_string(errno);
        emit q_ptr->error(characteristic);
        return false;
    } else {
        qCDebug(QT_BT_QNX) << "bt_gatt_enable_notify was presumably OK";
        return true;
    }
}

void QLowEnergyControllerPrivate::disableNotification(const QLowEnergyCharacteristicInfo &characteristic)
{
    int rc = bt_gatt_enable_notify(characteristic.d_ptr->instance, &characteristic.d_ptr->characteristic, 0);
    if (!rc)
        qCDebug(QT_BT_QNX) << "bt_gatt_enable_notify errno=" << errno << qt_error_string(errno);
    else
        qCDebug(QT_BT_QNX) << "bt_gatt_enable_notify was presumably OK";
}

bool QLowEnergyControllerPrivate::write(const QLowEnergyCharacteristicInfo &characteristic)
{
    if (!characteristic.isValid()) {
        errorString = QStringLiteral("Characteristic not valid.");
        emit q_ptr->error(characteristic);
        return false;
    }

    if (characteristic.permissions() & QLowEnergyCharacteristicInfo::Write) {
        writeValue(characteristic.d_ptr->instance, characteristic.d_ptr->handle, characteristic.d_ptr->value);
        if (errorString == QString()) {
            return true;
        } else {
            emit q_ptr->error(characteristic);
            return false;
        }
    } else {
        errorString = QStringLiteral("Characteristic does not allow write operations. The wanted value was not written to the device.");
        emit q_ptr->error(characteristic);
        return false;
    }

}

bool QLowEnergyControllerPrivate::write(const QLowEnergyDescriptorInfo &descriptor)
{
    Q_UNUSED(descriptor);
    return false;
}

void QLowEnergyControllerPrivate::readDescriptors(QLowEnergyCharacteristicInfo &characteristic)
{
    characteristic.d_ptr->descriptorsList.clear();
    int count = bt_gatt_descriptors_count(characteristic.d_ptr->instance, &characteristic.d_ptr->characteristic);

    if (count == -1) {
        qWarning() << "GATT descriptors count failed:" << errno << "(" << qt_error_string(errno) << ")";
        bt_gatt_disconnect_instance(characteristic.d_ptr->instance);
        return;
    }

    bt_gatt_descriptor_t *descriptorList = 0;
    if (count > 0) {
        descriptorList = (bt_gatt_descriptor_t*)alloca(count * sizeof(bt_gatt_descriptor_t));
        if (!descriptorList) {
            qCDebug(QT_BT_QNX) <<"GATT descriptors: Not enough memory";
            bt_gatt_disconnect_instance(characteristic.d_ptr->instance);
            return;
        }

        /* BEGIN WORKAROUND - Temporary fix to address race condition */
        int number = 0;
        do {
            number = bt_gatt_descriptors(characteristic.d_ptr->instance, &characteristic.d_ptr->characteristic, descriptorList, count);
        } while ((number == -1) && (errno == EBUSY));

        count = number;
        /* END WORKAROUND */
    }

    if (count == -1) {
        qCDebug(QT_BT_QNX) << "GATT descriptors failed: %1 (%2)" << errno << qt_error_string(errno);
        bt_gatt_disconnect_instance(characteristic.d_ptr->instance);
        return;
    }

    characteristic.d_ptr->characteristicMtu = bt_gatt_get_mtu(characteristic.d_ptr->instance);

    uint8_t *descriptorBuffer = (uint8_t *)alloca(characteristic.d_ptr->characteristicMtu);
    if (!descriptorBuffer) {
        qCDebug(QT_BT_QNX) <<"GATT descriptors: Not enough memory";
        bt_gatt_disconnect_instance(characteristic.d_ptr->instance);
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
            byteCount = bt_gatt_read_value(characteristic.d_ptr->instance, descriptorList[i].handle, offset, descriptorBuffer, characteristic.d_ptr->characteristicMtu, &more);
            if (byteCount < 0) {
                qCDebug(QT_BT_QNX) << "Unable to read descriptor value:"<< errno<< qt_error_string(errno);
                break;
            }
            descriptor.d_ptr->m_value = QByteArray();
            for (int j = 0; j < byteCount; j++) {
                QString hexadecimal;
                hexadecimal.setNum(descriptorBuffer[j], 16);
                descriptor.d_ptr->m_value.append(hexadecimal.toLatin1());
            }

        }
        descriptor.d_ptr->instance = characteristic.d_ptr->instance;
        map[QStringLiteral("value")] = descriptor.d_ptr->m_value;
        descriptor.d_ptr->m_properties = map;
        characteristic.d_ptr->descriptorsList.append(descriptor);
    }

    if (!descriptorList) {
        free(descriptorList);
        descriptorList = 0;
    }
}

void QLowEnergyControllerPrivate::readValue(QLowEnergyCharacteristicInfo &characteristic)
{
    if ((characteristic.d_ptr->permission & QLowEnergyCharacteristicInfo::Read) == 0) {
        qCDebug(QT_BT_QNX) << "GATT characteristic: Read not permitted";
        return;
    }

    uint8_t *characteristicBuffer = (uint8_t *)alloca(characteristic.d_ptr->characteristicMtu);
    if (!characteristicBuffer) {
        qCDebug(QT_BT_QNX) << "GATT characteristic: Not enough memory";
        bt_gatt_disconnect_instance(characteristic.d_ptr->instance);
        return;
    }

    int byteCount = 0;
    uint8_t more = 1;
    for (int offset = 0; more; offset += byteCount) {
        byteCount = bt_gatt_read_value(characteristic.d_ptr->instance,
                                       characteristic.d_ptr->handle.toUShort(), offset, characteristicBuffer,
                                       characteristic.d_ptr->characteristicMtu, &more);
        if (byteCount < 0) {
            qCDebug(QT_BT_QNX) << "Unable to read characteristic value: " << errno << qt_error_string(errno);
            break;
        }
        characteristic.d_ptr->value = QByteArray();
        for (int j = 0; j < byteCount; j++) {
            QString hexadecimal;
            hexadecimal.setNum(characteristicBuffer[j], 16);
            characteristic.d_ptr->value.append(hexadecimal.toLatin1());
        }
        characteristic.d_ptr->properties["value"] = characteristic.d_ptr->value;
    }
}

void QLowEnergyControllerPrivate::writeValue(const int &instance, const QString &handle, const QByteArray &value)
{
    errorString = QString();
    const int characteristicLen = value.size();
    uint8_t *characteristicBuffer = (uint8_t *)alloca(characteristicLen / 2 + 1);
    if (!characteristicBuffer) {
        qCDebug(QT_BT_QNX) << "GATT characteristic: Not enough memory";
        errorString = QStringLiteral("Not enough memory.");
        bt_gatt_disconnect_instance(instance);
        return;
    }

    const int consumed = stringToBuffer(QString::fromLocal8Bit(value), characteristicBuffer, characteristicLen);

    if (consumed > 0) {
        int byteCount;
        byteCount = bt_gatt_write_value(instance, handle.toUShort(), 0, characteristicBuffer, (consumed / 2));

        if (byteCount < 0) {
            qCDebug(QT_BT_QNX) << "Unable to write characteristic value: " << errno << qt_error_string(errno);
            errorString = QStringLiteral("Unable to write characteristic value: ") + qt_error_string(errno);
        }
    }
}

QT_END_NAMESPACE
