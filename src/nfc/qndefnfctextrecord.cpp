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

#include <qndefnfctextrecord.h>

#include <QtCore/QTextCodec>
#include <QtCore/QLocale>

QT_BEGIN_NAMESPACE

/*!
    \class QNdefNfcTextRecord
    \brief The QNdefNfcTextRecord class provides an NFC RTD-Text.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since 5.2

    RTD-Text encapsulates a user displayable text record.
*/

/*!
    \enum QNdefNfcTextRecord::Encoding

    This enum describes the text encoding standard used.

    \value Utf8     The text is encoded with UTF-8.
    \value Utf16    The text is encoding with UTF-16.
*/

/*!
    \fn QNdefNfcTextRecord::QNdefNfcTextRecord()

    Constructs an empty NFC text record of type \l QNdefRecord::NfcRtd.
*/

/*!
    \fn QNdefNfcTextRecord::QNdefNfcTextRecord(const QNdefRecord& other)

    Constructs a new NFC text record that is a copy of \a other.
*/

/*!
    Returns the locale of the text record.
*/
QString QNdefNfcTextRecord::locale() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QString();

    quint8 status = p.at(0);

    quint8 codeLength = status & 0x3f;

    return QString::fromLatin1(p.constData() + 1, codeLength);
}

/*!
    Sets the locale of the text record to \a locale.
*/
void QNdefNfcTextRecord::setLocale(const QString &locale)
{
    QByteArray p = payload();

    quint8 status = p.isEmpty() ? 0 : p.at(0);

    quint8 codeLength = status & 0x3f;

    quint8 newStatus = (status & 0xd0) | locale.length();

    p[0] = newStatus;
    p.replace(1, codeLength, locale.toLatin1());

    setPayload(p);
}

/*!
    Returns the contents of the text record as a string.
*/
QString QNdefNfcTextRecord::text() const
{
    const QByteArray p = payload();

    if (p.isEmpty())
        return QString();

#if QT_CONFIG(textcodec)
    quint8 status = p.at(0);

    bool utf16 = status & 0x80;
    quint8 codeLength = status & 0x3f;

    QTextCodec *codec = QTextCodec::codecForName(utf16 ? "UTF-16BE" : "UTF-8");

    return codec ? codec->toUnicode(p.constData() + 1 + codeLength, p.length() - 1 - codeLength) : QString();
#else
    qWarning("Cannot decode payload, Qt was built with -no-feature-textcodec!");
    return QString();
#endif
}

/*!
    Sets the contents of the text record to \a text.
*/
void QNdefNfcTextRecord::setText(const QString text)
{
#if QT_CONFIG(textcodec)
    if (payload().isEmpty())
        setLocale(QLocale().name());

    QByteArray p = payload();

    quint8 status = p.at(0);

    bool utf16 = status & 0x80;
    quint8 codeLength = status & 0x3f;

    p.truncate(1 + codeLength);

    QTextCodec *codec = QTextCodec::codecForName(utf16 ? "UTF-16BE" : "UTF-8");

    p += codec->fromUnicode(text);

    setPayload(p);
#else
    qWarning("Cannot encode payload, Qt was built with -no-feature-textcodec!");
    Q_UNUSED(text);
#endif
}

/*!
    Returns the encoding of the contents.
*/
QNdefNfcTextRecord::Encoding QNdefNfcTextRecord::encoding() const
{
    if (payload().isEmpty())
        return Utf8;

    QByteArray p = payload();

    quint8 status = p.at(0);

    bool utf16 = status & 0x80;

    if (utf16)
        return Utf16;
    else
        return Utf8;
}

/*!
    Sets the enconding of the contents to \a encoding.
*/
void QNdefNfcTextRecord::setEncoding(Encoding encoding)
{
    QByteArray p = payload();

    quint8 status = p.isEmpty() ? 0 : p.at(0);

    QString string = text();

    if (encoding == Utf8)
        status &= ~0x80;
    else
        status |= 0x80;

    p[0] = status;

    setPayload(p);

    setText(string);
}

QT_END_NAMESPACE
