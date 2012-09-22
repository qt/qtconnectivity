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


#ifndef QNEARFIELDTARGET_H
#define QNEARFIELDTARGET_H

#include "qnfcglobal.h"

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE
class QString;
class QUrl;
QT_END_NAMESPACE

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class QNdefMessage;
class QNearFieldTargetPrivate;

class Q_NFC_EXPORT QNearFieldTarget : public QObject
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QNearFieldTarget)

public:
    enum Type {
        AnyTarget,
        ProprietaryTag,
        NfcTagType1,
        NfcTagType2,
        NfcTagType3,
        NfcTagType4,
        MifareTag,
        NfcForumDevice
    };

    enum AccessMethod {
        NdefAccess,
        TagTypeSpecificAccess,
        LlcpAccess
    };
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
        NdefWriteError
    };

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

    explicit QNearFieldTarget(QObject *parent = 0);
    virtual ~QNearFieldTarget();

    virtual QByteArray uid() const = 0;
    virtual QUrl url() const;

    virtual Type type() const = 0;
    virtual AccessMethods accessMethods() const = 0;

    bool isProcessingCommand() const;

    // NdefAccess
    virtual bool hasNdefMessage();
    virtual RequestId readNdefMessages();
    virtual RequestId writeNdefMessages(const QList<QNdefMessage> &messages);

    // TagTypeSpecificAccess
    virtual RequestId sendCommand(const QByteArray &command);
    virtual RequestId sendCommands(const QList<QByteArray> &commands);

    virtual bool waitForRequestCompleted(const RequestId &id, int msecs = 5000);

    QVariant requestResponse(const RequestId &id);
    void setResponseForRequest(const QNearFieldTarget::RequestId &id, const QVariant &response,
                               bool emitRequestCompleted = true);

protected:
    Q_INVOKABLE virtual bool handleResponse(const QNearFieldTarget::RequestId &id,
                                            const QByteArray &response);

Q_SIGNALS:
    void disconnected();

    void ndefMessageRead(const QNdefMessage &message);
    void ndefMessagesWritten();

    void requestCompleted(const QNearFieldTarget::RequestId &id);

    void error(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);

private:
    QNearFieldTargetPrivate *d_ptr;
};

Q_NFC_EXPORT quint16 qNfcChecksum(const char * data, uint len);

Q_DECLARE_OPERATORS_FOR_FLAGS(QNearFieldTarget::AccessMethods)

QTNFC_END_NAMESPACE

Q_DECLARE_METATYPE(QtNfc::QNearFieldTarget::RequestId)
Q_DECLARE_METATYPE(QtNfc::QNearFieldTarget::Error)

QT_END_HEADER

#endif // QNEARFIELDTARGET_H
