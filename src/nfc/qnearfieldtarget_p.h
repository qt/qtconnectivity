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

#ifndef QNEARFIELDTARGET_P_H
#define QNEARFIELDTARGET_P_H

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

#include <QtCore/QMap>
#include <QtCore/QSharedData>
#include <QtCore/QVariant>

#define NEARFIELDTARGET_Q() NearFieldTarget * const q = reinterpret_cast<NearFieldTarget *>(q_ptr)

QT_BEGIN_NAMESPACE

class QNearFieldTarget::RequestIdPrivate : public QSharedData
{
};

class QNearFieldTargetPrivate
{
    QNearFieldTarget *q_ptr;
    Q_DECLARE_PUBLIC(QNearFieldTarget)

public:
    QNearFieldTargetPrivate(QNearFieldTarget *q) : q_ptr(q) {}

    QMap<QNearFieldTarget::RequestId, QVariant> m_decodedResponses;

    bool keepConnection() const;
    bool setKeepConnection(bool isPersistent);
    bool disconnect();
    int maxCommandLength() const;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_P_H
