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

#include "qnearfieldtagtype4_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QNearFieldTagType4
    \brief The QNearFieldTagType4 class provides an interface for communicating with an NFC Tag
           Type 4 tag.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \internal
*/

/*!
    \fn Type QNearFieldTagType4::type() const
    \reimp
*/

/*!
    Constructs a new tag type 4 near field target with \a parent.
*/
QNearFieldTagType4::QNearFieldTagType4(QObject *parent)
:   QNearFieldTarget(parent)
{
}

/*!
    Destroys the tag type 4 near field target.
*/
QNearFieldTagType4::~QNearFieldTagType4()
{
}

/*!
    Returns the NFC Tag Type 4 specification version number that the tag supports.
*/
quint8 QNearFieldTagType4::version()
{
    return 0;
}

/*!
    Requests that the file specified by \a name be selected. Upon success calls to read() and
    write() will act on the selected file. Returns a request id which can be used to track the
    completion status of the request.

    Once the request completes the response can be retrieved from the requestResponse() function.
    The response of this request will be a boolean value, true for success; otherwise false.

    \sa requestCompleted(), waitForRequestCompleted()
*/
QNearFieldTarget::RequestId QNearFieldTagType4::select(const QByteArray &name)
{
    Q_UNUSED(name);

    return RequestId();
}

/*!
    Requests that the file specified by \a fileIdentifier be selected. Upon success calls to read()
    and write() will act on the selected file. Returns a request id which can be used to track the
    completion status of the request.

    Once the request completes the response can be retrieved from the requestResponse() function.
    The response of this request will be a boolean value, true for success; otherwise false.

    \sa requestCompleted(), waitForRequestCompleted()
*/
QNearFieldTarget::RequestId QNearFieldTagType4::select(quint16 fileIdentifier)
{
    Q_UNUSED(fileIdentifier);

    return RequestId();
}

/*!
    Requests that \a length bytes be read from the currently selected file starting from
    \a startOffset. If \a length is 0 all data or the maximum read size bytes will be read,
    whichever is smaller. Returns a request id which can be used to track the completion status of
    the request.

    Once the request completes successfully the response can be retrieved from the
    requestResponse() function. The response of this request will be a QByteArray.

    \sa requestCompleted(), waitForRequestCompleted()
*/
QNearFieldTarget::RequestId QNearFieldTagType4::read(quint16 length, quint16 startOffset)
{
    Q_UNUSED(length);
    Q_UNUSED(startOffset);

    return RequestId();
}

/*!
    Writes \a data to the currently selected file starting at \a startOffset. Returns a request id
    which can be used to track the completion status of the request.

    Once the request completes the response can be retrieved from the requestResponse() function.
    The response of this request will be a boolean value, true for success; otherwise false.

    \sa requestCompleted(), waitForRequestCompleted()
*/
QNearFieldTarget::RequestId QNearFieldTagType4::write(const QByteArray &data, quint16 startOffset)
{
    Q_UNUSED(data);
    Q_UNUSED(startOffset);

    return RequestId();
}

/*!
    \reimp
*/
bool QNearFieldTagType4::handleResponse(const QNearFieldTarget::RequestId &id,
                                        const QByteArray &response)
{
    return QNearFieldTarget::handleResponse(id, response);
}

QT_END_NAMESPACE
