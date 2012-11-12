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

    void setAction(QNdefNfcSmartPosterRecord::ActionValue actionValue);
    QNdefNfcSmartPosterRecord::ActionValue action() const;
};

class QNdefNfcIconRecord : public QNdefRecord
{
public:
    Q_DECLARE_MIME_NDEF_RECORD(QNdefNfcIconRecord, QByteArray(0, char(0)))

    void setData(const QByteArray& data);
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
