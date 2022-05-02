/****************************************************************************
**
** Copyright (C) 2020 Governikus GmbH & Co. KG
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
******************************************************************************/

#ifndef QNEARFIELDTARGET_IOS_P_H
#define QNEARFIELDTARGET_IOS_P_H

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

#include "qnearfieldtarget_p.h"

#include <QQueue>
#include <QTimer>
#include <memory>

QT_BEGIN_NAMESPACE

class ResponseProvider : public QObject
{
    Q_OBJECT

    public:
        void provideResponse(QNearFieldTarget::RequestId requestId, bool success, QByteArray recvBuffer);

    Q_SIGNALS:
        void responseReceived(QNearFieldTarget::RequestId requestId, bool success, QByteArray recvBuffer);
};

struct NfcTagDeleter
{
    void operator()(void *tag);
};

class QNearFieldTargetPrivateImpl : public QNearFieldTargetPrivate
{
    Q_OBJECT

public:
    QNearFieldTargetPrivateImpl(void *tag, QObject *parent = nullptr);
    ~QNearFieldTargetPrivateImpl() override;
    void invalidate();

    QByteArray uid() const override;
    QNearFieldTarget::Type type() const override;
    QNearFieldTarget::AccessMethods accessMethods() const override;

    int maxCommandLength() const override;
    QNearFieldTarget::RequestId sendCommand(const QByteArray &command) override;

    bool isAvailable() const;

Q_SIGNALS:
    void targetLost(QNearFieldTargetPrivateImpl *target);

private:
    std::unique_ptr<void, NfcTagDeleter> nfcTag;
    bool connected = false;
    QTimer targetCheckTimer;
    QNearFieldTarget::RequestId requestInProgress;
    QQueue<std::pair<QNearFieldTarget::RequestId, QByteArray>> queue;

    bool connect();

private Q_SLOTS:
    void onTargetCheck();
    void onTargetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);
    void onExecuteRequest();
    void onResponseReceived(QNearFieldTarget::RequestId requestId, bool success, QByteArray recvBuffer);
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_IOS_P_H
