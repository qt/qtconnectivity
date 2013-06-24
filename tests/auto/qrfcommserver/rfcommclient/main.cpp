/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

#include <QCoreApplication>
#include <QStringList>
#include "rfcommclient.h"
#include <qbluetoothdeviceinfo.h>
//#include <qbluetoothlocaldevice.h>
//#include <QtTest/QtTest>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    RfCommClient client;
    QBluetoothLocalDevice localDevice;
    MyThread mythread;

    QObject::connect(&client, SIGNAL(done()), &app, SLOT(quit()));

    QString address;
    QString port;
    QStringList args = QCoreApplication::arguments();

    if (args.length() >= 2){
        address = args.at(1);
        if (args.length() >= 3){
            port = args.at(2);
        }
    }

       // use previous value for client, stored earlier
//    if (address.isEmpty()){
//        QSettings settings("QtDF", "bttennis");
//        address = settings.value("lastclient").toString();
//    }

    // hard-code address and port number if not provided
    if (address.isEmpty()){
        address = "6C:9B:02:0C:91:D3";  // "J C7-2"
        port = QString("20");
    }

    if (!address.isEmpty()){
        qDebug() << "Connecting to" << address << port;
        QBluetoothDeviceInfo device = QBluetoothDeviceInfo(QBluetoothAddress(address), "",
                QBluetoothDeviceInfo::MiscellaneousDevice);
        QBluetoothServiceInfo service;
        if (!port.isEmpty()) {
            QBluetoothServiceInfo::Sequence protocolDescriptorList;
            QBluetoothServiceInfo::Sequence protocol;
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
                     << QVariant::fromValue(port.toUShort());
            protocolDescriptorList.append(QVariant::fromValue(protocol));
            service.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                                 protocolDescriptorList);
            qDebug() << "port" << port.toUShort() << service.protocolServiceMultiplexer();
        }
        else {
            service.setServiceUuid(QBluetoothUuid(serviceUuid));
        }
        service.setDevice(device);
        // delay so that server is in waiting state
        qDebug() << "Starting sleep";
        mythread.sleep(10);  // seconds
        qDebug() << "Finished sleeping";
        client.startClient(service);
    } else {
        qDebug() << "failed because address and/or port is missing " << address << port;
    }

    return app.exec();
}

