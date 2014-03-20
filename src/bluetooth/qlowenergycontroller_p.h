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
#ifndef QLOWENERGYCONTROLLER_P_H
#define QLOWENERGYCONTROLLER_P_H
#include "qlowenergycontroller.h"

QT_BEGIN_NAMESPACE

class QLowEnergyProcess;

class QLowEnergyControllerPrivate
{
    Q_DECLARE_PUBLIC(QLowEnergyController)
public:
    QLowEnergyControllerPrivate();
    ~QLowEnergyControllerPrivate();
    void connectService(const QLowEnergyServiceInfo &service);
    void disconnectService(const QLowEnergyServiceInfo &leService = QLowEnergyServiceInfo());
    bool enableNotification(const QLowEnergyCharacteristicInfo &characteristic);
    void disableNotification(const QLowEnergyCharacteristicInfo &characteristic);
    bool write(const QLowEnergyCharacteristicInfo &characteristic);
    bool write(const QLowEnergyDescriptorInfo &descriptor);
    void disconnectAllServices();

    QList<QLowEnergyServiceInfo> m_leServices;
    QString errorString;
    QLowEnergyController::Error error;

#ifdef QT_QNX_BLUETOOTH
    static void serviceConnected(const char*, const char*, int, int, short unsigned int, short unsigned int, short unsigned int, void*);
    static void serviceUpdate(const char *, int , short unsigned int, short unsigned int, short unsigned int, void *);
    static void serviceDisconnected(const char *, const char *, int, int, void *);
    static void serviceNotification(int, short unsigned int, const char unsigned *, short unsigned int, void *);
    void readDescriptors(QLowEnergyCharacteristicInfo &characteristic);
    void readValue(QLowEnergyCharacteristicInfo &characteristic);
    void writeValue(const int &instance, const QString &handle, const QByteArray &value);
#endif

#ifdef QT_BLUEZ_BLUETOOTH
    void connectToTerminal();
    void setHandles();
    void setCharacteristics(int);
    void setNotifications();
    void readCharacteristicValue(int);
    void writeValue(const QString &, const QByteArray &);
public slots:
    void _q_replyReceived(const QString &reply);
#endif
private:
    bool m_randomAddress;
    QLowEnergyProcess *process;
    QBluetoothAddress localAdapter;
#ifdef QT_BLUEZ_BLUETOOTH
    int m_step;
    bool m_deviceConnected;
    bool m_commandStarted;
#endif
    QLowEnergyController *q_ptr;
};
QT_END_NAMESPACE
#endif // QLOWENERGYCONTROLLER_P_H
