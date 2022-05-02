/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef QNEARFIELDTARGET_H
#define QNEARFIELDTARGET_H

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QVariant>
#include <QtNfc/qtnfcglobal.h>

QT_BEGIN_NAMESPACE

class QNdefMessage;
class QNearFieldTargetPrivate;

class Q_NFC_EXPORT QNearFieldTarget : public QObject
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QNearFieldTarget)

public:
    enum Type {
        ProprietaryTag,
        NfcTagType1,
        NfcTagType2,
        NfcTagType3,
        NfcTagType4,
        NfcTagType4A,
        NfcTagType4B,
        MifareTag
    };
    Q_ENUM(Type)

    enum AccessMethod {
        UnknownAccess = 0x00,
        NdefAccess = 0x01,
        TagTypeSpecificAccess = 0x02,
        AnyAccess = 0xff
    };
    Q_ENUM(AccessMethod)
    Q_DECLARE_FLAGS(AccessMethods, AccessMethod)

    enum Error {
        NoError,
        UnknownError,
        UnsupportedError,
        TargetOutOfRangeError,
        NoResponseError,
        ChecksumMismatchError,
        InvalidParametersError,
        ConnectionError,
        NdefReadError,
        NdefWriteError,
        CommandError,
        TimeoutError
    };
    Q_ENUM(Error)

    class RequestIdPrivate;
    class Q_NFC_EXPORT RequestId
    {
    public:
        RequestId();
        RequestId(const RequestId &other);
        RequestId(RequestIdPrivate *p);
        ~RequestId();

        bool isValid() const;

        int refCount() const;

        bool operator<(const RequestId &other) const;
        bool operator==(const RequestId &other) const;
        bool operator!=(const RequestId &other) const;
        RequestId &operator=(const RequestId &other);

        QSharedDataPointer<RequestIdPrivate> d;
    };

    explicit QNearFieldTarget(QObject *parent = nullptr);
    ~QNearFieldTarget();

    QByteArray uid() const;
    Type type() const;
    AccessMethods accessMethods() const;

    bool disconnect();

    // NdefAccess
    bool hasNdefMessage();
    RequestId readNdefMessages();
    RequestId writeNdefMessages(const QList<QNdefMessage> &messages);

    // TagTypeSpecificAccess
    int maxCommandLength() const;
    RequestId sendCommand(const QByteArray &command);

    bool waitForRequestCompleted(const RequestId &id, int msecs = 5000);
    QVariant requestResponse(const RequestId &id) const;

Q_SIGNALS:
    void disconnected();

    void ndefMessageRead(const QNdefMessage &message);

    void requestCompleted(const QNearFieldTarget::RequestId &id);

    void error(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);

protected:
    QNearFieldTarget(QNearFieldTargetPrivate *backend, QObject *parent = nullptr);

private:
    QNearFieldTargetPrivate *d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QNearFieldTarget::AccessMethods)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QNearFieldTarget::RequestId)

#endif // QNEARFIELDTARGET_H
