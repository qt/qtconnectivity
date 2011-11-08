/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVEBLUETOOTHSERVICE_P_H
#define QDECLARATIVEBLUETOOTHSERVICE_P_H

#include <QObject>
#include <qdeclarative.h>
#include <qbluetoothserviceinfo.h>
#include <qdeclarativebluetoothsocket_p.h>

class QDeclarativeBluetoothSocket;

QTBLUETOOTH_USE_NAMESPACE

class QDeclarativeBluetoothServicePrivate;

class QDeclarativeBluetoothService : public QObject, public QDeclarativeParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY detailsChanged)
    Q_PROPERTY(QString deviceAddress READ deviceAddress WRITE setDeviceAddress NOTIFY detailsChanged)
    Q_PROPERTY(QString serviceName READ serviceName WRITE setServiceName NOTIFY detailsChanged)
    Q_PROPERTY(QString serviceDescription READ serviceDescription WRITE setServiceDescription NOTIFY detailsChanged)
    Q_PROPERTY(QString serviceUuid READ serviceUuid WRITE setServiceUuid NOTIFY detailsChanged)
    Q_PROPERTY(QString serviceProtocol READ serviceProtocol WRITE setServiceProtocol NOTIFY detailsChanged)
    Q_PROPERTY(qint32 servicePort READ servicePort WRITE setServicePort NOTIFY detailsChanged)
    Q_PROPERTY(bool registered READ isRegistered WRITE setRegistered NOTIFY registeredChanged)

    Q_INTERFACES(QDeclarativeParserStatus)

public:
    explicit QDeclarativeBluetoothService(QObject *parent = 0);
    explicit QDeclarativeBluetoothService(const QBluetoothServiceInfo &service,
                                          QObject *parent = 0);
    ~QDeclarativeBluetoothService();

    QString deviceName() const;
    QString deviceAddress() const;
    QString serviceName() const;
    QString serviceDescription() const;
    QString serviceUuid() const;
    QString serviceProtocol() const;
    qint32 servicePort() const;
    bool isRegistered() const;

    QBluetoothServiceInfo *serviceInfo() const;

    Q_INVOKABLE QDeclarativeBluetoothSocket *nextClient();
    Q_INVOKABLE void assignNextClient(QDeclarativeBluetoothSocket *dbs);

    // From QDeclarativeParserStatus
    void classBegin() {}
    void componentComplete();

signals:
    void detailsChanged();
    void registeredChanged();
    void newClient();

public slots:
    void setServiceName(QString name);
    void setDeviceAddress(QString address);
    void setServiceDescription(QString description);
    void setServiceUuid(QString uuid);
    void setServiceProtocol(QString protocol);
    void setServicePort(qint32 port);
    void setRegistered(bool registered);


private slots:
    void new_connection();

private:
    QDeclarativeBluetoothServicePrivate* d;
    friend class QDeclarativeBluetoothServicePrivate;

};

QML_DECLARE_TYPE(QDeclarativeBluetoothService)

#endif // QDECLARATIVEBLUETOOTHSERVICE_P_H
