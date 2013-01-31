#include "qnxnfceventfilter_p.h"
#include <QDebug>
#include "nfc/nfc.h"
#include "qnxnfcmanager_p.h"

QT_BEGIN_NAMESPACE_NFC

QNXNFCEventFilter::QNXNFCEventFilter()
{
}

void QNXNFCEventFilter::installOnEventDispatcher(QAbstractEventDispatcher *dispatcher)
{
    dispatcher->installNativeEventFilter(this);
    // start flow of navigator events
    navigator_request_events(0);
}

void QNXNFCEventFilter::uninstallEventFilter()
{
    removeEventFilter(this);
}

bool QNXNFCEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);
    bps_event_t *event = static_cast<bps_event_t *>(message);

    int code = bps_event_get_code(event);

    if (code == NAVIGATOR_INVOKE_TARGET) {
        // extract bps request from event
        const navigator_invoke_invocation_t *invoke = navigator_invoke_event_get_invocation(event);
        const char *uri = navigator_invoke_invocation_get_uri(invoke);
        const char *type = navigator_invoke_invocation_get_type(invoke);
        int dataLength = navigator_invoke_invocation_get_data_length(invoke);
        const char *raw_data = (const char*)navigator_invoke_invocation_get_data(invoke);
        QByteArray data(raw_data, dataLength);

        //message.fromByteArray(data);

        const char* metadata = navigator_invoke_invocation_get_metadata(invoke);

        nfc_ndef_message_t *ndefMessage;
        nfc_create_ndef_message_from_bytes(reinterpret_cast<const uchar_t *>(data.data()),
                                                  data.length(), &ndefMessage);

        QNdefMessage message = QNXNFCManager::instance()->decodeMessage(ndefMessage);

        unsigned int ndefRecordCount;
        nfc_get_ndef_record_count(ndefMessage, &ndefRecordCount);

        qQNXNFCDebug() << "Got Invoke event" << uri << "Type" << type;

        Q_EMIT ndefEvent(message);
    }

    return false;
}

QT_END_NAMESPACE_NFC
