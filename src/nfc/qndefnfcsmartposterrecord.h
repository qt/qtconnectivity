#ifndef QNDEFNFCSMARTPOSTERRECORD_H
#define QNDEFNFCSMARTPOSTERRECORD_H

#include <QtNfc/qnfcglobal.h>
#include <QtNfc/QNdefRecord>
#include <qndefnfctextrecord.h>

QT_FORWARD_DECLARE_CLASS(QUrl)

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class Q_NFC_EXPORT QNdefNfcSmartPosterRecord : public QNdefRecord
{
public:
    enum ActionValue {
        Unset = -1,
        Do = 0,
        Save = 1,
        Open = 2
    };

    Q_DECLARE_NDEF_RECORD(QNdefNfcSmartPosterRecord, QNdefRecord::NfcRtd, "Sp", QByteArray(0, char(0)))

    bool hasTitle(const QString& locale) const;
    bool hasAction() const;
    bool hasIcon(const QByteArray& mimetype) const;
    bool hasSize() const;
    bool hasTypeInfo() const;

    QString title(const QString& locale) const;
    QString titleLocale(const QString& locale) const;
    QNdefNfcTextRecord::Encoding titleEncoding(const QString& locale) const;

    bool addTitle(const QString& text, const QString& locale, QNdefNfcTextRecord::Encoding encoding);

    QUrl uri() const;
    bool setUri(const QUrl& url);

    ActionValue action() const;
    bool setAction(ActionValue act);

    QByteArray icon(const QByteArray& mimetype) const;
    QByteArray iconType(const QByteArray& mimetype) const;

    QByteArray icon(int index) const;
    QByteArray iconType(int index) const;
    quint32 iconCount() const;

    void addIcon(const QByteArray& type, const QByteArray& data);

    quint32 size() const;
    bool setSize(quint32 size);

    QByteArray typeInfo() const;
    bool setTypeInfo(const QByteArray& type);
};

QTNFC_END_NAMESPACE

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(QNdefNfcSmartPosterRecord, QNdefRecord::NfcRtd, "Sp")

QT_END_HEADER

#endif // QNDEFNFCSMARTPOSTERRECORD_H
