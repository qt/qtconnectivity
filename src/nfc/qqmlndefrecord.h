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

#ifndef QQMLNDEFRECORD_H
#define QQMLNDEFRECORD_H

#include <QtCore/QObject>
#include <QtCore/QMetaType>
#include <QtNfc/QNdefRecord>

QT_BEGIN_NAMESPACE

class QQmlNdefRecordPrivate;

class Q_NFC_EXPORT QQmlNdefRecord : public QObject
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QQmlNdefRecord)

    Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(TypeNameFormat typeNameFormat READ typeNameFormat WRITE setTypeNameFormat NOTIFY typeNameFormatChanged)
    Q_PROPERTY(QNdefRecord record READ record WRITE setRecord NOTIFY recordChanged)

public:
    enum TypeNameFormat {
        Empty = QNdefRecord::Empty,
        NfcRtd = QNdefRecord::NfcRtd,
        Mime = QNdefRecord::Mime,
        Uri = QNdefRecord::Uri,
        ExternalRtd = QNdefRecord::ExternalRtd,
        Unknown = QNdefRecord::Unknown
    };
    Q_ENUM(TypeNameFormat)

    explicit QQmlNdefRecord(QObject *parent = nullptr);
    explicit QQmlNdefRecord(const QNdefRecord &record, QObject *parent = nullptr);
    ~QQmlNdefRecord();

    QString type() const;
    void setType(const QString &t);

    void setTypeNameFormat(TypeNameFormat typeNameFormat);
    TypeNameFormat typeNameFormat() const;

    QNdefRecord record() const;
    void setRecord(const QNdefRecord &record);

Q_SIGNALS:
    void typeChanged();
    void typeNameFormatChanged();
    void recordChanged();

private:
    QQmlNdefRecordPrivate *d_ptr;
};

void Q_NFC_EXPORT qRegisterNdefRecordTypeHelper(const QMetaObject *metaObject,
                                                         QNdefRecord::TypeNameFormat typeNameFormat,
                                                         const QByteArray &type);

Q_NFC_EXPORT QQmlNdefRecord *qNewDeclarativeNdefRecordForNdefRecord(const QNdefRecord &record);

template<typename T>
bool qRegisterNdefRecordType(QNdefRecord::TypeNameFormat typeNameFormat, const QByteArray &type)
{
    qRegisterNdefRecordTypeHelper(&T::staticMetaObject, typeNameFormat, type);
    return true;
}

#define Q_DECLARE_NDEFRECORD(className, typeNameFormat, type) \
static bool _q_##className##_registered = qRegisterNdefRecordType<className>(typeNameFormat, type);

QT_END_NAMESPACE

#endif // QQMLNDEFRECORD_H
