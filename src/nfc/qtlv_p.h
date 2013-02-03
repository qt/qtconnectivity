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

#ifndef QTLV_P_H
#define QTLV_P_H

#include "qnfcglobal.h"

#include "qnearfieldtarget.h"

#include <QtCore/QByteArray>
#include <QtCore/QMap>
#include <QtCore/QPair>

QT_BEGIN_NAMESPACE_NFC

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

QT_END_NAMESPACE_NFC

#endif // QTLV_P_H
