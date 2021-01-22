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

#ifndef QNEARFIELDTARGET_H
#define QNEARFIELDTARGET_H

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>
#include <QtNfc/qtnfcglobal.h>

QT_BEGIN_NAMESPACE
class QString;
class QUrl;
QT_END_NAMESPACE

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
        MifareTag
    };
    Q_ENUM(Type)

    enum AccessMethod {
        UnknownAccess = 0x00,
        NdefAccess = 0x01,
        TagTypeSpecificAccess = 0x02,
        LlcpAccess = 0x04
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
        NdefReadError,
        NdefWriteError,
        CommandError
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
    virtual ~QNearFieldTarget();

    virtual QByteArray uid() const = 0;
    virtual QUrl url() const;

    virtual Type type() const = 0;
    virtual AccessMethods accessMethods() const = 0;

    bool keepConnection() const;
    bool setKeepConnection(bool isPersistent);
    bool disconnect();

    bool isProcessingCommand() const;

    // NdefAccess
    virtual bool hasNdefMessage();
    virtual RequestId readNdefMessages();
    virtual RequestId writeNdefMessages(const QList<QNdefMessage> &messages);

    // TagTypeSpecificAccess
    int maxCommandLength() const;
    virtual RequestId sendCommand(const QByteArray &command);
    virtual RequestId sendCommands(const QList<QByteArray> &commands);

    virtual bool waitForRequestCompleted(const RequestId &id, int msecs = 5000);

    QVariant requestResponse(const RequestId &id);
    void setResponseForRequest(const QNearFieldTarget::RequestId &id, const QVariant &response,
                               bool emitRequestCompleted = true);

protected:
    Q_INVOKABLE virtual bool handleResponse(const QNearFieldTarget::RequestId &id,
                                            const QByteArray &response);

    void reportError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);

Q_SIGNALS:
    void disconnected();

    void ndefMessageRead(const QNdefMessage &message);
    void ndefMessagesWritten();

    void requestCompleted(const QNearFieldTarget::RequestId &id);

    void error(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);

private:
    QNearFieldTargetPrivate *d_ptr;
};

#if QT_DEPRECATED_SINCE(5, 9)
Q_NFC_EXPORT quint16 qNfcChecksum(const char * data, uint len);
#endif

Q_DECLARE_OPERATORS_FOR_FLAGS(QNearFieldTarget::AccessMethods)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QNearFieldTarget::RequestId)

#endif // QNEARFIELDTARGET_H
