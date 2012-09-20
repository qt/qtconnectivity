/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QNEARFIELDTAGTYPE1_H
#define QNEARFIELDTAGTYPE1_H

#include <qnearfieldtarget.h>

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class QNearFieldTagType1Private;

class Q_NFC_EXPORT QNearFieldTagType1 : public QNearFieldTarget
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QNearFieldTagType1)

public:
    enum WriteMode {
        EraseAndWrite,
        WriteOnly
    };

    explicit QNearFieldTagType1(QObject *parent = 0);
    ~QNearFieldTagType1();

    Type type() const { return NfcTagType1; }

    bool hasNdefMessage();
    RequestId readNdefMessages();
    RequestId writeNdefMessages(const QList<QNdefMessage> &messages);

    quint8 version();
    virtual int memorySize();

    // DIGPROTO
    virtual RequestId readIdentification();

    // static memory functions
    virtual RequestId readAll();
    virtual RequestId readByte(quint8 address);
    virtual RequestId writeByte(quint8 address, quint8 data, WriteMode mode = EraseAndWrite);

    // dynamic memory functions
    virtual RequestId readSegment(quint8 segmentAddress);
    virtual RequestId readBlock(quint8 blockAddress);
    virtual RequestId writeBlock(quint8 blockAddress, const QByteArray &data,
                                 WriteMode mode = EraseAndWrite);

protected:
    bool handleResponse(const QNearFieldTarget::RequestId &id, const QByteArray &response);

private:
    QNearFieldTagType1Private *d_ptr;
};

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif // QNEARFIELDTAGTYPE1_H
