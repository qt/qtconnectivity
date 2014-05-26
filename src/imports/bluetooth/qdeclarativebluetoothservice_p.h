/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QDECLARATIVEBLUETOOTHSERVICE_P_H
#define QDECLARATIVEBLUETOOTHSERVICE_P_H

#include <QObject>
#include <qqml.h>
#include <qbluetoothserviceinfo.h>
#include "qdeclarativebluetoothsocket_p.h"

class QDeclarativeBluetoothSocket;

QT_USE_NAMESPACE

class QDeclarativeBluetoothServicePrivate;

class QDeclarativeBluetoothService : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY detailsChanged)
    Q_PROPERTY(QString deviceAddress READ deviceAddress WRITE setDeviceAddress NOTIFY detailsChanged)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName NOTIFY detailsChanged)
    Q_PROPERTY(QString serviceDescription READ serviceDescription WRITE setServiceDescription NOTIFY detailsChanged)
    Q_PROPERTY(QString serviceUuid READ serviceUuid WRITE setServiceUuid NOTIFY detailsChanged)
    Q_PROPERTY(Protocol serviceProtocol READ serviceProtocol WRITE setServiceProtocol NOTIFY detailsChanged)
    Q_PROPERTY(bool registered READ isRegistered WRITE setRegistered NOTIFY registeredChanged)

    Q_INTERFACES(QQmlParserStatus)
    Q_ENUMS(Protocol)

public:
    /// TODO: Merge/Replace with QBluetoothServiceInfo::Protocol in Qt 6
    enum Protocol {
        RfcommProtocol = QBluetoothServiceInfo::RfcommProtocol,
        L2CapProtocol = QBluetoothServiceInfo::L2capProtocol,
        UnknownProtocol = QBluetoothServiceInfo::UnknownProtocol
    };

    explicit QDeclarativeBluetoothService(QObject *parent = 0);
    explicit QDeclarativeBluetoothService(const QBluetoothServiceInfo &service,
                                          QObject *parent = 0);
    ~QDeclarativeBluetoothService();

    QString deviceName() const;
    QString deviceAddress() const;
    QString serviceName() const;
    QString serviceDescription() const;
    QString serviceUuid() const;
    Protocol serviceProtocol() const;
    bool isRegistered() const;

    QBluetoothServiceInfo *serviceInfo() const;

    Q_INVOKABLE QDeclarativeBluetoothSocket *nextClient();
    Q_INVOKABLE void assignNextClient(QDeclarativeBluetoothSocket *dbs);

    // From QDeclarativeParserStatus
    void classBegin() {}
    void componentComplete();

    void setServiceName(const QString &name);
    void setDeviceAddress(const QString &address);
    void setServiceDescription(const QString &description);
    void setServiceUuid(const QString &uuid);
    void setServiceProtocol(QDeclarativeBluetoothService::Protocol protocol);
    void setRegistered(bool registered);

signals:
    void detailsChanged();
    void registeredChanged();
    void newClient();

private slots:
    void new_connection();

private:
    QDeclarativeBluetoothServicePrivate* d;
    friend class QDeclarativeBluetoothServicePrivate;

};

QML_DECLARE_TYPE(QDeclarativeBluetoothService)

#endif // QDECLARATIVEBLUETOOTHSERVICE_P_H
