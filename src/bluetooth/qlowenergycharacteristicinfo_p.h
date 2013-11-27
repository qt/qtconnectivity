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

#ifndef QLOWENERGYCHARACTERISTICINFO_P_H
#define QLOWENERGYCHARACTERISTICINFO_P_H
#include "qbluetoothuuid.h"
#include "qlowenergycharacteristicinfo.h"

#ifdef QT_BLUEZ_BLUETOOTH
#include <QtDBus/QtDBus>
#include <QObject>
#endif
#ifdef QT_QNX_BLUETOOTH
#include <btapi/btdevice.h>
#include <btapi/btgatt.h>
#include <btapi/btspp.h>
#include <btapi/btle.h>
#endif
#ifdef QT_BLUEZ_BLUETOOTH
class OrgBluezCharacteristicInterface;
class QLowEnergyProcess;
#endif

QT_BEGIN_NAMESPACE
class QBluetoothUuid;
class QLowEnergyCharacteristicInfo;

class QLowEnergyCharacteristicInfoPrivate: public QObject
{
    Q_OBJECT
public:
    QLowEnergyCharacteristicInfoPrivate();
    ~QLowEnergyCharacteristicInfoPrivate();

    void setValue(const QByteArray &wantedValue);
    void readValue();
    bool valid();
    bool enableNotification();
    void disableNotification();
    void readDescriptors();

    QString name;
    QBluetoothUuid uuid;
    QByteArray value;
    int permission;
    bool notification;
    QString handle;
    QString notificationHandle;
    int instance;
    QVariantMap properties;
    QString errorString;
    QList<QLowEnergyDescriptorInfo> descriptorsList;
#ifdef QT_QNX_BLUETOOTH
    bt_gatt_characteristic_t characteristic;
    int characteristicMtu;
    bt_gatt_char_prop_mask characteristicProperties;
    static void serviceNotification(int, short unsigned int, const unsigned char*, short unsigned int, void *);
#endif
#ifdef QT_BLUEZ_BLUETOOTH
    QString path;
    int t;
    QString startingHandle;
public Q_SLOTS:

    void replyReceived(const QString &reply);
#endif

Q_SIGNALS:
    void notifyValue(const QBluetoothUuid &);
    void error(const QBluetoothUuid &);

private:
#ifdef QT_BLUEZ_BLUETOOTH
    OrgBluezCharacteristicInterface *characteristic;
    QLowEnergyProcess *process;
    bool m_signalConnected;
#endif

};
QT_END_NAMESPACE

#endif // QLOWENERGYCHARACTERISTICINFO_P_H
