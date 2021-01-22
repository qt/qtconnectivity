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

#ifndef QNEARFIELDTAGTYPE4_H
#define QNEARFIELDTAGTYPE4_H

#include <QtNfc/QNearFieldTarget>

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

QT_BEGIN_NAMESPACE

class Q_AUTOTEST_EXPORT QNearFieldTagType4 : public QNearFieldTarget
{
    Q_OBJECT

public:
    explicit QNearFieldTagType4(QObject *parent = 0);
    ~QNearFieldTagType4();

    Type type() const { return NfcTagType4; }

    quint8 version();

    virtual RequestId select(const QByteArray &name);
    virtual RequestId select(quint16 fileIdentifier);

    virtual RequestId read(quint16 length = 0, quint16 startOffset = 0);
    virtual RequestId write(const QByteArray &data, quint16 startOffset = 0);

protected:
    bool handleResponse(const QNearFieldTarget::RequestId &id, const QByteArray &response);
};

QT_END_NAMESPACE

#endif // QNEARFIELDTAGTYPE4_H
