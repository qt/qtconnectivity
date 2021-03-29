/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
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

#include "qnearfieldtarget.h"

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QSharedData>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

class QNearFieldTarget::RequestIdPrivate : public QSharedData
{
};

class Q_AUTOTEST_EXPORT QNearFieldTargetPrivate : public QObject
{
    Q_OBJECT

public:
    QNearFieldTarget *q_ptr;

    explicit QNearFieldTargetPrivate(QObject *parent = nullptr);
    virtual ~QNearFieldTargetPrivate() = default;

    virtual QByteArray uid() const;
    virtual QNearFieldTarget::Type type() const;
    virtual QNearFieldTarget::AccessMethods accessMethods() const;

    virtual bool disconnect();

    // NdefAccess
    virtual bool hasNdefMessage();
    virtual QNearFieldTarget::RequestId readNdefMessages();
    virtual QNearFieldTarget::RequestId writeNdefMessages(const QList<QNdefMessage> &messages);

    // TagTypeSpecificAccess
    virtual int maxCommandLength() const;
    virtual QNearFieldTarget::RequestId sendCommand(const QByteArray &command);

    bool waitForRequestCompleted(const QNearFieldTarget::RequestId &id, int msecs = 5000);
    QVariant requestResponse(const QNearFieldTarget::RequestId &id) const;

Q_SIGNALS:
    void disconnected();

    void ndefMessageRead(const QNdefMessage &message);

    void requestCompleted(const QNearFieldTarget::RequestId &id);

    void error(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);

protected:
    QMap<QNearFieldTarget::RequestId, QVariant> m_decodedResponses;

    virtual void setResponseForRequest(const QNearFieldTarget::RequestId &id,
                                       const QVariant &response,
                                       bool emitRequestCompleted = true);

    void reportError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);
};

class NearFieldTarget : public QNearFieldTarget
{
public:
    NearFieldTarget(QNearFieldTargetPrivate *backend, QObject *parent = nullptr)
    : QNearFieldTarget(backend, parent)
    {
        backend->q_ptr = this;
        backend->setParent(this);
    }
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_P_H
