#ifndef BBNFCMANAGER_H
#define BBNFCMANAGER_H

#include <QObject>
#include "nfc/nfc_types.h"
#include "nfc/nfc.h"
#include <QDebug>
#include <QSocketNotifier>
#include "../qndefmessage.h"
#include "../qndefrecord.h"
//#include <bb/system/InvokeManager>
//#include <bb/system/InvokeRequest>
//#include <bb/system/ApplicationStartupMode>
//#include "../qnearfieldtarget_blackberry_p.h"

#ifdef QNXNFC_DEBUG
#define qQNXNFCDebug qDebug
#else
#define qQNXNFCDebug QT_NO_QDEBUG_MACRO
#endif

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class Q_DECL_EXPORT QNXNFCManager : public QObject
{
    Q_OBJECT
public:
    static QNXNFCManager *instance();
    void registerForNewInstance();
    void unregisterForInstance();
    nfc_target_t *getLastTarget();
    bool isAvailable();

private:
    QNXNFCManager();
    ~QNXNFCManager();

    static QNXNFCManager *m_instance;
    int m_instanceCount;

    int nfcFD;
    QSocketNotifier *nfcNotifier;

    QList<QNdefMessage> decodeTargetMessage(nfc_target_t *);

    QList<QPair<QObject*, QMetaMethod> > ndefMessageHandlers;

    //There can only be one target. The last detected one is saved here
    //currently we do not get notified when the target is disconnected. So the target might be invalid
    nfc_target_t *m_lastTarget;
    bool m_available;

public Q_SLOTS:
    void newNfcEvent(int fd);
    void nfcReadWriteEvent(nfc_event_t *nfcEvent);
    void startBTHandover();
    //TODO add a parameter to only detect a special target for now we are detecting all target types
    bool startTargetDetection();
    //void handleInvoke(const bb::system::InvokeRequest& request);

Q_SIGNALS:
    //void llcpEvent();
    //void ndefMessage(const QNdefMessage&);
    //void targetDetected(NearFieldTarget*, const QList<QNdefMessage>&);
    //Not sure if this is implementable
    void targetLost(); //Not available yet
    //void bluetoothHandover();
};

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif

