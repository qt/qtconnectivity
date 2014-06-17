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

#ifndef QLOWENERGYCONTROLLERNEWPRIVATE_P_H
#define QLOWENERGYCONTROLLERNEWPRIVATE_P_H

#include <qglobal.h>
#include <QtBluetooth/qbluetooth.h>
#include "qlowenergycontrollernew.h"

#if defined(QT_BLUEZ_BLUETOOTH) && !defined(QT_BLUEZ_NO_BTLE)
#include <QtBluetooth/QBluetoothSocket>
#endif

QT_BEGIN_NAMESPACE

typedef QPair<QLowEnergyHandle,QLowEnergyHandle> HandlePair;

struct ServiceDetails {
    QLowEnergyHandle startHandle, endHandle;
    QSharedPointer<QLowEnergyService> service;
};

class QLowEnergyControllerNewPrivate : QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QLowEnergyControllerNew)
public:
    QLowEnergyControllerNewPrivate()
        : QObject(),
          state(QLowEnergyControllerNew::UnconnectedState),
          error(QLowEnergyControllerNew::NoError)
#if defined(QT_BLUEZ_BLUETOOTH) && !defined(QT_BLUEZ_NO_BTLE)
          , l2cpSocket(0)
#endif
    {}

    void setError(QLowEnergyControllerNew::Error newError);
    void validateLocalAdapter();
    void setState(QLowEnergyControllerNew::ControllerState newState);

    void connectToDevice();
    void disconnectFromDevice();

    void discoverServices();

    void discoverServiceDetails(const QBluetoothUuid &);

    QBluetoothAddress remoteDevice;
    QBluetoothAddress localAdapter;

    QLowEnergyControllerNew::ControllerState state;
    QLowEnergyControllerNew::Error error;
    QString errorString;


private:
    // list of all found service uuids
    QMap<QBluetoothUuid, ServiceDetails> serviceList;

#if defined(QT_BLUEZ_BLUETOOTH) && !defined(QT_BLUEZ_NO_BTLE)
    QBluetoothSocket *l2cpSocket;

    void sendReadByGroupRequest(QLowEnergyHandle start, QLowEnergyHandle end);

private slots:
    void l2cpConnected();
    void l2cpDisconnected();
    void l2cpErrorChanged(QBluetoothSocket::SocketError);
    void l2cpReadyRead();
#endif
private:
    QLowEnergyControllerNew *q_ptr;
};

QT_END_NAMESPACE

#endif // QLOWENERGYCONTROLLERNEWPRIVATE_P_H
