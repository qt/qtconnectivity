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

#ifndef QLOWENERGYSERVICEINFO_P_H
#define QLOWENERGYSERVICEINFO_P_H
#include "qbluetoothuuid.h"
#include "qlowenergyserviceinfo.h"
#include "qlowenergycharacteristicinfo.h"
#include <QPointer>

QT_BEGIN_NAMESPACE

class QBluetoothUuid;
class QLowEnergyServiceInfo;
class QLowEnergyCharacteristicInfo;
class QLowEnergyProcess;

class QLowEnergyServiceInfoPrivate
{
    friend class QLowEnergyControllerPrivate;

public:
    QLowEnergyServiceInfoPrivate();
    ~QLowEnergyServiceInfoPrivate();

    QString serviceName;

    QBluetoothUuid uuid;

    QList<QLowEnergyCharacteristicInfo> characteristicList;
    QLowEnergyServiceInfo::ServiceType serviceType;
    bool connected;
    QBluetoothDeviceInfo deviceInfo;
#ifdef QT_BLUEZ_BLUETOOTH
    QString startingHandle;
    QString endingHandle;
#endif

#ifdef QT_QNX_BLUETOOTH
    static void serviceNotification(int, short unsigned int, const char unsigned *, short unsigned int, void *);
#endif

private:
#ifdef QT_BLUEZ_BLUETOOTH
    int m_step;
    int m_valueCounter;
    int m_readCounter;
#endif
};

QT_END_NAMESPACE

#endif // QLOWENERGYSERVICEINFO_P_H
