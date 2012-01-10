/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTLV_P_H
#define QTLV_P_H

#include "qnfcglobal.h"

#include "qnearfieldtarget.h"

#include <QtCore/QByteArray>
#include <QtCore/QMap>
#include <QtCore/QPair>

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class QNearFieldTarget;
class Q_AUTOTEST_EXPORT QTlvReader
{
public:
    explicit QTlvReader(QNearFieldTarget *target);
    explicit QTlvReader(const QByteArray &data);

    void addReservedMemory(int offset, int length);
    int reservedMemorySize() const;

    QNearFieldTarget::RequestId requestId() const;

    bool atEnd() const;

    bool readNext();

    quint8 tag() const;
    int length();
    QByteArray data();

private:
    bool readMoreData(int sparseOffset);
    int absoluteOffset(int sparseOffset) const;
    int dataLength(int startOffset) const;

    QNearFieldTarget *m_target;
    QByteArray m_rawData;
    QNearFieldTarget::RequestId m_requestId;

    QByteArray m_tlvData;
    int m_index;
    QMap<int, int> m_reservedMemory;
};

class QTlvWriter
{
public:
    explicit QTlvWriter(QNearFieldTarget *target);
    explicit QTlvWriter(QByteArray *data);
    ~QTlvWriter();

    void addReservedMemory(int offset, int length);

    void writeTlv(quint8 tag, const QByteArray &data = QByteArray());

    bool process(bool all = false);

    QNearFieldTarget::RequestId requestId() const;

private:
    int moveToNextAvailable();

    QNearFieldTarget *m_target;
    QByteArray *m_rawData;

    int m_index;
    int m_tagMemorySize;
    QMap<int, int> m_reservedMemory;

    QByteArray m_buffer;

    QNearFieldTarget::RequestId m_requestId;
};

QPair<int, int> qParseReservedMemoryControlTlv(const QByteArray &tlvData);
QPair<int, int> qParseLockControlTlv(const QByteArray &tlvData);

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif // QTLV_P_H
