#ifndef QNXNFCEVENTFILTER_H
#define QNXNFCEVENTFILTER_H

#include <QAbstractNativeEventFilter>
#include <QAbstractEventDispatcher>
//#include "qnxnfcmanager_p.h"
#include <bps/navigator.h>
#include "bps/bps.h"
#include "bps/navigator_invoke.h"
#include "../qnfcglobal.h"
#include "../qndefmessage.h"

QTNFC_BEGIN_NAMESPACE

class QNXNFCEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    QNXNFCEventFilter();

    void installOnEventDispatcher(QAbstractEventDispatcher *dispatcher);
    void uninstallEventFilter();
private:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

    QAbstractNativeEventFilter *prevFilter;

Q_SIGNALS:
    void ndefEvent(const QNdefMessage &msg);
};

QTNFC_END_NAMESPACE

#endif // QNXNFCEVENTFILTER_H
