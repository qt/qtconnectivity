/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QTLV_P_H
#define QTLV_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qtnfcglobal.h"

#include "qnearfieldtarget.h"

#include <QtCore/QByteArray>
#include <QtCore/QMap>
#include <QtCore/QPair>

QT_BEGIN_NAMESPACE

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

QT_END_NAMESPACE

#endif // QTLV_P_H
