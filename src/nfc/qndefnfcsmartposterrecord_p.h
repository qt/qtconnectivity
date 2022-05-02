/***************************************************************************
 **
 ** Copyright (C) 2016 - 2012 Research In Motion
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
 ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **********************************************************************/

#ifndef QNDEFNFCSMARTPOSTERRECORD_P_H
#define QNDEFNFCSMARTPOSTERRECORD_P_H

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

class QNdefNfcActRecord : public QNdefRecord
{
public:
    Q_DECLARE_NDEF_RECORD(QNdefNfcActRecord, QNdefRecord::NfcRtd, "act", QByteArray(0, char(0)))

    void setAction(QNdefNfcSmartPosterRecord::Action action);
    QNdefNfcSmartPosterRecord::Action action() const;
};

class QNdefNfcSizeRecord : public QNdefRecord
{
public:
    Q_DECLARE_NDEF_RECORD(QNdefNfcSizeRecord, QNdefRecord::NfcRtd, "s", QByteArray(0, char(0)))

    void setSize(quint32 size);
    quint32 size() const;
};

class QNdefNfcTypeRecord : public QNdefRecord
{
public:
    Q_DECLARE_NDEF_RECORD(QNdefNfcTypeRecord, QNdefRecord::NfcRtd, "t", QByteArray(0, char(0)))

    void setTypeInfo(const QString &type);
    QString typeInfo() const;
};

class QNdefNfcSmartPosterRecordPrivate : public QSharedData
{
public:
    QNdefNfcSmartPosterRecordPrivate() {}

public:
    QList<QNdefNfcTextRecord> m_titleList;
    QNdefNfcUriRecord *m_uri = nullptr;
    QNdefNfcActRecord *m_action = nullptr;
    QList<QNdefNfcIconRecord> m_iconList;
    QNdefNfcSizeRecord *m_size = nullptr;
    QNdefNfcTypeRecord *m_type = nullptr;
};

QT_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcActRecord, QNdefRecord::NfcRtd, "act")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcSizeRecord, QNdefRecord::NfcRtd, "s")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcTypeRecord, QNdefRecord::NfcRtd, "t")

#endif // QNDEFNFCSMARTPOSTERRECORD_P_H
