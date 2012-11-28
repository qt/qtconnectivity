/***************************************************************************
 **
 ** Copyright (C) 2011 - 2012 Research In Motion
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the plugins of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in13	
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

#ifndef QNDEFNFCSMARTPOSTERRECORD_P_H
#define QNDEFNFCSMARTPOSTERRECORD_P_H

QTNFC_BEGIN_NAMESPACE

#define Q_DECLARE_ISRECORDTYPE_FOR_MIME_NDEF_RECORD(className) \
    QTNFC_BEGIN_NAMESPACE \
    template<> inline bool QNdefRecord::isRecordType<className>() const\
    { \
        return (typeNameFormat() == QNdefRecord::Mime); \
    } \
    QTNFC_END_NAMESPACE

#define Q_DECLARE_MIME_NDEF_RECORD(className, initialPayload) \
    className() : QNdefRecord(QNdefRecord::Mime, "") { setPayload(initialPayload); } \
    className(const QNdefRecord &other) : QNdefRecord(other, QNdefRecord::Mime) { }

class QNdefNfcActRecord : public QNdefRecord
{
public:
    Q_DECLARE_NDEF_RECORD(QNdefNfcActRecord, QNdefRecord::NfcRtd, "act", QByteArray(0, char(0)))

    void setAction(QNdefNfcSmartPosterRecord::Action action);
    QNdefNfcSmartPosterRecord::Action action() const;
};

class QNdefNfcIconRecord : public QNdefRecord
{
public:
    Q_DECLARE_MIME_NDEF_RECORD(QNdefNfcIconRecord, QByteArray(0, char(0)))

    void setData(const QByteArray &data);
    QByteArray data() const;
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

    void setTypeInfo(const QByteArray &type);
    QByteArray typeInfo() const;
};

QTNFC_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcActRecord, QNdefRecord::NfcRtd, "act")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcSizeRecord, QNdefRecord::NfcRtd, "s")
Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcTypeRecord, QNdefRecord::NfcRtd, "t")
Q_DECLARE_ISRECORDTYPE_FOR_MIME_NDEF_RECORD(QNdefNfcIconRecord)

#endif // QNDEFNFCSMARTPOSTERRECORD_P_H
