/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QL2CAPSOCKET_H
#define QL2CAPSOCKET_H

#include <qbluetoothsocket.h>
#include <qbluetoothaddress.h>

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE_BLUETOOTH

class QL2capSocket : public QBluetoothSocket
{
    Q_OBJECT

public:
    explicit QL2capSocket(QObject *parent = 0);

    bool hasPendingDatagrams() const;
    qint64 pendingDatagramSize() const;
    qint64 readDatagram(char *data, qint64 maxlen, QBluetoothAddress *host = 0, quint16 *port = 0);
    qint64 writeDatagram(const char *data, qint64 len, const QBluetoothAddress &host,
                         quint16 port);
    inline qint64 writeDatagram(const QByteArray &datagram, const QBluetoothAddress &host,
                                quint16 port)
        { return writeDatagram(datagram.constData(), datagram.size(), host, port); }

private:
    Q_DISABLE_COPY(QL2capSocket)
};

QT_END_NAMESPACE_BLUETOOTH

#endif // QL2CAPSOCKET_H
