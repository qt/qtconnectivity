
/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
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

#include "qlowenergyserviceinfo_p.h"
#include "qlowenergycharacteristicinfo.h"
#include "qlowenergycharacteristicinfo_p.h"
#include "qlowenergyprocess_p.h"
#include <btapi/btdevice.h>
#include <btapi/btgatt.h>
#include <btapi/btspp.h>
#include <btapi/btle.h>
#include <errno.h>
#include <QPointer>
#include "qnx/ppshelpers_p.h"

QT_BEGIN_NAMESPACE

void QLowEnergyServiceInfoPrivate::serviceConnected(const char *bdaddr, const char *service, int instance, int err, short unsigned int connInt, short unsigned int latency, short unsigned int superTimeout, void *userData)
{
    Q_UNUSED(latency);
    Q_UNUSED(connInt);
    Q_UNUSED(superTimeout);
    QPointer<QLowEnergyServiceInfoPrivate> *classPointer = static_cast<QPointer<QLowEnergyServiceInfoPrivate> *>(userData);
    QLowEnergyServiceInfoPrivate *p = classPointer->data();
    qBBBluetoothDebug() << "---------------------------------------------------";
    qBBBluetoothDebug() << "[SERVICE: Connected] (service uuid, instance):" << p->uuid << instance;
    qBBBluetoothDebug() << "[SERVICE: Connected] Device address: " << bdaddr;
    qBBBluetoothDebug() << "[SERVICE: Connected] Device service: " << service;
    qBBBluetoothDebug() << "[SERVICE: Connected] Possible error: " << err;
    if (err != 0) {
        qBBBluetoothDebug() << "An error occurred in service connected callback: " << strerror(err);
        p->errorString = QString::fromLatin1(strerror(err));
        p->error(p->uuid);
    }
    p->characteristicList.clear();
    bt_gatt_characteristic_t* data;
    data = (bt_gatt_characteristic_t*) malloc(sizeof(bt_gatt_characteristic_t));
    if (0 == data) {
        qBBBluetoothDebug() << "[SERVICE: Connected] GATT characteristics: Not enough memory";
        bt_gatt_disconnect_instance(instance);
        p->errorString = QStringLiteral("GATT characteristics: Not enough memory");
        p->error(p->uuid);
        return;
    }

    int num_characteristics = bt_gatt_characteristics_count(instance);

    if (num_characteristics > -1) {
        qBBBluetoothDebug() << "Characteristics number: "<< num_characteristics;
        bt_gatt_characteristic_t *allCharacteristicList;

        allCharacteristicList = (bt_gatt_characteristic_t*) malloc(num_characteristics * sizeof(bt_gatt_characteristic_t));
        if (0 == allCharacteristicList) {
            qBBBluetoothDebug() <<" GATT characteristics: Not enough memory";
            bt_gatt_disconnect_instance(instance);
            p->errorString = QStringLiteral("GATT characteristics: Not enough memory");
            p->error(p->uuid);
            return;
        }

        /* BEGIN WORKAROUND - Temporary fix to address race condition */
        int number = 0;
        do {
            number = bt_gatt_characteristics(instance, allCharacteristicList, num_characteristics);
        } while ((number == -1) && (errno== EBUSY));
        //Using sub to subscribe notification callback only once
        bool sub = false;
        int characteristicListSize = number;

        for (int i = 0; i < characteristicListSize; i++) {
            qBBBluetoothDebug() << "Characteristic: uuid,handle,value_handle, properties:" << allCharacteristicList[i].uuid << "," << allCharacteristicList[i].handle << "," << allCharacteristicList[i].value_handle << ", " << allCharacteristicList[i].properties;
            QString charUuid = QString::fromLatin1(allCharacteristicList[i].uuid);
            QString handleUuid;
            handleUuid.setNum(allCharacteristicList[i].value_handle);
            QBluetoothUuid characteristicUuid;
            if (charUuid.toUShort(0,0) == 0) {
                charUuid = charUuid.remove(0,2);
                characteristicUuid = QBluetoothUuid(charUuid);
            }
            else
                characteristicUuid = QBluetoothUuid(charUuid.toUShort(0,0));
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
            characteristicInfo.d_ptr->readDescriptors();

            characteristicInfo.d_ptr->readValue();
            //Subscribe only once since it is static function
            if (sub == false) {
                int rc = bt_gatt_reg_notifications(instance, &(characteristicInfo.d_ptr->serviceNotification));
                if (rc != 0) {
                    qBBBluetoothDebug() << "[SERVICE: Connected] bt_gatt_reg_notifications failed." << errno << strerror(errno);
                    p->errorString = QString::fromLatin1(strerror(errno));
                    p->error(p->uuid);
                }
                else
                    qBBBluetoothDebug() << "[SERVICE: Connected] bt_gatt_reg_notifications was presumably OK";
                sub = true;
            }
            p->characteristicList.append(characteristicInfo);

        }

        if (allCharacteristicList != NULL) {
            free(allCharacteristicList);
            allCharacteristicList = NULL;
        }

        /* END WORKAROUND */
    }

    p->connected = true;
    qBBBluetoothDebug() << p;
    emit p->connectedToService(p->uuid);
    qBBBluetoothDebug() << "---------------------------------------------------------------------------------";
}

void QLowEnergyServiceInfoPrivate::serviceUpdate(const char *bdaddr, int instance, short unsigned int connInt, short unsigned int latency, short unsigned int superTimeout, void *userData)
{
    Q_UNUSED(latency);
    Q_UNUSED(connInt);
    Q_UNUSED(superTimeout);
    Q_UNUSED(userData);
    qBBBluetoothDebug() << "---------------------------------------------------";
    qBBBluetoothDebug() << "[SERVICE: Update] (instance):" << instance;
    qBBBluetoothDebug() << "[SERVICE: Update] Device address: " << bdaddr;
    qBBBluetoothDebug() << "---------------------------------------------------";
}

void QLowEnergyServiceInfoPrivate::serviceDisconnected(const char *bdaddr, const char *service, int instance, int reason, void *userData)
{
    QPointer<QLowEnergyServiceInfoPrivate> *classPointer = static_cast<QPointer<QLowEnergyServiceInfoPrivate> *>(userData);
    QLowEnergyServiceInfoPrivate *p = classPointer->data();
    emit p->disconnectedFromService(p->uuid);
    qBBBluetoothDebug() << "---------------------------------------------------";
    qBBBluetoothDebug() << "[SERVICE: Disconnect] (service, instance, reason):" << service << instance << reason;
    qBBBluetoothDebug() << "[SERVICE: Disconnect] Device address: " << bdaddr;
    qBBBluetoothDebug() << "---------------------------------------------------";
    delete p;
    delete classPointer;
}

QLowEnergyServiceInfoPrivate::QLowEnergyServiceInfoPrivate():
    errorString(QString()), m_instance(0)
{
    qRegisterMetaType<QBluetoothUuid>("QBluetoothUuid");
}

QLowEnergyServiceInfoPrivate::~QLowEnergyServiceInfoPrivate()
{

}

void QLowEnergyServiceInfoPrivate::registerServiceWatcher()
{
    bt_gatt_callbacks_t gatt_callbacks = { &(this->serviceConnected), this->serviceDisconnected, this->serviceUpdate };
    errno=0;
    process = process->instance();
    if (!process->isConnected()) {
        qBBBluetoothDebug() << "[INIT] Init problem." << errno << strerror(errno);
        errorString = QStringLiteral("Initialization of device falied. ") + QString::fromLatin1(strerror(errno));
        emit error(uuid);
        return;
    }

    errno=0;
    if (bt_gatt_init(&gatt_callbacks) < 0) {
        qBBBluetoothDebug() << "[INIT] GAT Init problem." << errno << strerror(errno);
        errorString = QStringLiteral("Callbacks initialization failed. ") + QString::fromLatin1(strerror(errno));
        emit error(uuid);
        return;
    }
    if (bt_le_init(0) != EOK) {
        qWarning() << "LE initialization failure " << errno;
    }

    QPointer<QLowEnergyServiceInfoPrivate> *classPointer = new QPointer<QLowEnergyServiceInfoPrivate>(this);
    process->addPointer(classPointer->data());
    QString serviceUuid = uuid.toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
    if (serviceUuid.contains(QStringLiteral("000000000000")))
        serviceUuid = QStringLiteral("0x") + serviceUuid.toUpper();
    else
        serviceUuid = QStringLiteral("0x") + serviceUuid[4] + serviceUuid[5] + serviceUuid[6] + serviceUuid[7];
    errno=0;
    bt_gatt_conn_parm_t conParm;
    conParm.minConn = 0x30;
    conParm.maxConn = 0x50;
    conParm.latency = 0;
    conParm.superTimeout = 50;
    if (bt_gatt_connect_service(deviceInfo.address().toString().toLocal8Bit().constData(), serviceUuid.toLocal8Bit().constData(), 0, &conParm, classPointer) < 0) {
        qBBBluetoothDebug() << "[SERVICE] Connection to service failed." << errno << strerror(errno);
        errorString = QStringLiteral("[SERVICE] Connection to service failed.") + QString::fromLatin1(strerror(errno));
        emit error(uuid);
    }
    qBBBluetoothDebug() << "errno after connect: " << errno;
}

void QLowEnergyServiceInfoPrivate::unregisterServiceWatcher()
{
    QString serviceUuid = uuid.toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
    if (serviceUuid.contains(QStringLiteral("000000000000")))
        serviceUuid = QStringLiteral("0x") + serviceUuid.toUpper();
    else
        serviceUuid = QStringLiteral("0x") + serviceUuid[4] + serviceUuid[5] + serviceUuid[6] + serviceUuid[7];
    if (bt_gatt_disconnect_service(deviceInfo.address().toString().toLocal8Bit().constData(), serviceUuid.toLocal8Bit().constData()) < 0) {
            qBBBluetoothDebug() << "[SERVICE] Disconnect service request failed. " << errno << strerror(errno);
            emit error(uuid);
        } else {
            emit disconnectedFromService(uuid);
            qBBBluetoothDebug() << "[SERVICE] Disconnected from service OK.";
    }
}

bool QLowEnergyServiceInfoPrivate::valid()
{
    return true;
}

QT_END_NAMESPACE
