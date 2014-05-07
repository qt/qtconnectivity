/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include <QtCore/QGlobalStatic>
#include <QtCore/QLoggingCategory>
#include <QtCore/QMap>
#include "bluez5_helper_p.h"
#include "objectmanager_p.h"
#include "properties_p.h"
#include "adapter1_bluez5_p.h"
#include <bluetooth/sdp_lib.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

typedef enum Bluez5TestResultType
{
    Unknown,
    Bluez4,
    Bluez5
} Bluez5TestResult;

Q_GLOBAL_STATIC_WITH_ARGS(Bluez5TestResult, bluezVersion, (Bluez5TestResult::Unknown));

bool isBluez5()
{
    if (*bluezVersion() == Bluez5TestResultType::Unknown) {
        OrgFreedesktopDBusObjectManagerInterface manager(QStringLiteral("org.bluez"),
                                                         QStringLiteral("/"),
                                                         QDBusConnection::systemBus());

        qDBusRegisterMetaType<InterfaceList>();
        qDBusRegisterMetaType<ManagedObjectList>();

        QDBusPendingReply<ManagedObjectList> reply = manager.GetManagedObjects();
        reply.waitForFinished();
        if (reply.isError()) {
            *bluezVersion() = Bluez5TestResultType::Bluez4;
            qCDebug(QT_BT_BLUEZ) << "Bluez 4 detected.";
        } else {
            *bluezVersion() = Bluez5TestResultType::Bluez5;
            qCDebug(QT_BT_BLUEZ) << "Bluez 5 detected.";
        }
    }

    return (*bluezVersion() == Bluez5TestResultType::Bluez5);
}

struct AdapterData
{
public:
    AdapterData() : reference(1), wasListeningAlready(false), propteryListener(0) {}

    int reference;
    bool wasListeningAlready;
    OrgFreedesktopDBusPropertiesInterface *propteryListener;
};

class QtBluezDiscoveryManagerPrivate
{
public:
    QMap<QString, AdapterData *> references;
    OrgFreedesktopDBusObjectManagerInterface *manager;
};

Q_GLOBAL_STATIC(QtBluezDiscoveryManager, discoveryManager)

/*!
    \internal
    \class QtBluezDiscoveryManager

    This class manages the access to "org.bluez.Adapter1::Start/StopDiscovery.

    The flag is a system flag. We want to ensure that the various Qt classes don't turn
    the flag on and off and thereby get into their way. If some other system component
    changes the flag (e.g. adapter removed) we notify all Qt classes about the change by emitting
    \l discoveryInterrupted(QString). Classes should indicate this via an appropriate
    error message to the user.

    Once the signal was emitted, all existing requests for discovery mode on the same adapter
    have to be renewed via \l registerDiscoveryInterest(QString).
*/

QtBluezDiscoveryManager::QtBluezDiscoveryManager(QObject *parent) :
    QObject(parent)
{
    qCDebug(QT_BT_BLUEZ) << "Creating QtBluezDiscoveryManager";
    d = new QtBluezDiscoveryManagerPrivate();

    d->manager = new OrgFreedesktopDBusObjectManagerInterface(
                QStringLiteral("org.bluez"), QStringLiteral("/"),
                QDBusConnection::systemBus(), this);
    connect(d->manager, SIGNAL(InterfacesRemoved(QDBusObjectPath,QStringList)),
            SLOT(InterfacesRemoved(QDBusObjectPath,QStringList)));
}

QtBluezDiscoveryManager::~QtBluezDiscoveryManager()
{
    qCDebug(QT_BT_BLUEZ) << "Destroying QtBluezDiscoveryManager";

    foreach (const QString &adapterPath, d->references.keys()) {
        AdapterData *data = d->references.take(adapterPath);
        delete data->propteryListener;

        // turn discovery off if it wasn't on already
        if (!data->wasListeningAlready) {
            OrgBluezAdapter1Interface iface(QStringLiteral("org.bluez"), adapterPath,
                                            QDBusConnection::systemBus());
            iface.StopDiscovery();
        }

        delete data;
    }

    delete d;
}

QtBluezDiscoveryManager *QtBluezDiscoveryManager::instance()
{
    if (isBluez5())
        return discoveryManager();

    Q_ASSERT(false);
    return 0;
}

bool QtBluezDiscoveryManager::registerDiscoveryInterest(const QString &adapterPath)
{
    if (adapterPath.isEmpty())
        return false;

    // already monitored adapter? -> increase ref count -> done
    if (d->references.contains(adapterPath)) {
        d->references[adapterPath]->reference++;
        return true;
    }

    AdapterData *data = new AdapterData();

    OrgFreedesktopDBusPropertiesInterface *propIface = new OrgFreedesktopDBusPropertiesInterface(
                QStringLiteral("org.bluez"), adapterPath, QDBusConnection::systemBus());
    connect(propIface, SIGNAL(PropertiesChanged(QString,QVariantMap,QStringList)),
            SLOT(PropertiesChanged(QString,QVariantMap,QStringList)));
    data->propteryListener = propIface;

    OrgBluezAdapter1Interface iface(QStringLiteral("org.bluez"), adapterPath,
                                    QDBusConnection::systemBus());
    data->wasListeningAlready = iface.discovering();

    d->references[adapterPath] = data;

    if (!data->wasListeningAlready)
        iface.StartDiscovery();

    return true;
}

void QtBluezDiscoveryManager::unregisterDiscoveryInterest(const QString &adapterPath)
{
    if (!d->references.contains(adapterPath))
        return;

    AdapterData *data = d->references[adapterPath];
    data->reference--;

    if (data->reference > 0) // more than one client requested discovery mode
        return;

    d->references.remove(adapterPath);
    if (!data->wasListeningAlready) { // Qt turned discovery mode on, Qt has to turn it off again
        OrgBluezAdapter1Interface iface(QStringLiteral("org.bluez"), adapterPath,
                                        QDBusConnection::systemBus());
        iface.StopDiscovery();
    }

    delete data->propteryListener;
    delete data;
}

//void QtBluezDiscoveryManager::dumpState() const
//{
//    qCDebug(QT_BT_BLUEZ) << "-------------------------";
//    if (d->references.isEmpty()) {
//        qCDebug(QT_BT_BLUEZ) << "No running registration";
//    } else {
//        foreach (const QString &path, d->references.keys()) {
//            qCDebug(QT_BT_BLUEZ) << path << "->" << d->references[path]->reference;
//        }
//    }
//    qCDebug(QT_BT_BLUEZ) << "-------------------------";
//}

void QtBluezDiscoveryManager::InterfacesRemoved(const QDBusObjectPath &object_path,
                                                const QStringList &interfaces)
{
    if (!d->references.contains(object_path.path())
            || !interfaces.contains(QStringLiteral("org.bluez.Adapter1")))
        return;

    removeAdapterFromMonitoring(object_path.path());
}

void QtBluezDiscoveryManager::PropertiesChanged(const QString &interface,
                                                const QVariantMap &changed_properties,
                                                const QStringList &invalidated_properties)
{
    Q_UNUSED(invalidated_properties);

    OrgFreedesktopDBusPropertiesInterface *propIface =
            qobject_cast<OrgFreedesktopDBusPropertiesInterface *>(sender());

    if (!propIface)
        return;

    if (interface == QStringLiteral("org.bluez.Adapter1")
            && d->references.contains(propIface->path())
            && changed_properties.contains(QStringLiteral("Discovering"))) {
        bool isDiscovering = changed_properties.value(QStringLiteral("Discovering")).toBool();
        if (!isDiscovering)
            removeAdapterFromMonitoring(propIface->path());
    }
}

void QtBluezDiscoveryManager::removeAdapterFromMonitoring(const QString &dbusPath)
{
    // remove adapter from monitoring
    AdapterData *data = d->references.take(dbusPath);
    delete data->propteryListener;
    delete data;

    emit discoveryInterrupted(dbusPath);
}

#define BUFFER_SIZE 1024

static void parseAttributeValues(sdp_data_t *data, int indentation, QByteArray &xmlOutput)
{
    if (!data)
        return;

    const int length = indentation*2 + 1;
    QByteArray indentString(length, ' ');

    char snBuffer[BUFFER_SIZE];

    xmlOutput.append(indentString);

    // deal with every dtd type
    switch (data->dtd) {
    case SDP_DATA_NIL:
        xmlOutput.append("<nil/>\n");
        break;
    case SDP_UINT8:
        qsnprintf(snBuffer, BUFFER_SIZE, "<uint8 value=\"0x%02x\"/>\n", data->val.uint8);
        xmlOutput.append(snBuffer);
        break;
    case SDP_UINT16:
        qsnprintf(snBuffer, BUFFER_SIZE, "<uint16 value=\"0x%04x\"/>\n", data->val.uint16);
        xmlOutput.append(snBuffer);
        break;
    case SDP_UINT32:
        qsnprintf(snBuffer, BUFFER_SIZE, "<uint32 value=\"0x%08x\"/>\n", data->val.uint32);
        xmlOutput.append(snBuffer);
        break;
    case SDP_UINT64:
        qsnprintf(snBuffer, BUFFER_SIZE, "<uint64 value=\"0x%016x\"/>\n", data->val.uint64);
        xmlOutput.append(snBuffer);
        break;
    case SDP_UINT128:
        xmlOutput.append("<uint128 value=\"0x");
        for (int i = 0; i < 16; i++)
            ::sprintf(&snBuffer[i * 2], "%02x", data->val.uint128.data[i]);
        xmlOutput.append(snBuffer);
        xmlOutput.append("\"/>\n");
        break;
    case SDP_INT8:
        qsnprintf(snBuffer, BUFFER_SIZE, "<int8 value=\"%d\"/>/n", data->val.int8);
        xmlOutput.append(snBuffer);
        break;
    case SDP_INT16:
        qsnprintf(snBuffer, BUFFER_SIZE, "<int16 value=\"%d\"/>/n", data->val.int16);
        xmlOutput.append(snBuffer);
        break;
    case SDP_INT32:
        qsnprintf(snBuffer, BUFFER_SIZE, "<int32 value=\"%d\"/>/n", data->val.int32);
        xmlOutput.append(snBuffer);
        break;
    case SDP_INT64:
        qsnprintf(snBuffer, BUFFER_SIZE, "<int64 value=\"%d\"/>/n", data->val.int64);
        xmlOutput.append(snBuffer);
        break;
    case SDP_INT128:
        xmlOutput.append("<int128 value=\"0x");
        for (int i = 0; i < 16; i++)
            ::sprintf(&snBuffer[i * 2], "%02x", data->val.int128.data[i]);
        xmlOutput.append(snBuffer);
        xmlOutput.append("\"/>\n");
        break;
    case SDP_UUID_UNSPEC:
        break;
    case SDP_UUID16:
    case SDP_UUID32:
        xmlOutput.append("<uuid value=\"0x");
        sdp_uuid2strn(&(data->val.uuid), snBuffer, BUFFER_SIZE);
        xmlOutput.append(snBuffer);
        xmlOutput.append("\"/>\n");
        break;
    case SDP_UUID128:
        xmlOutput.append("<uuid value=\"");
        sdp_uuid2strn(&(data->val.uuid), snBuffer, BUFFER_SIZE);
        xmlOutput.append(snBuffer);
        xmlOutput.append("\"/>\n");
        break;
    case SDP_TEXT_STR_UNSPEC:
        break;
    case SDP_TEXT_STR8:
    case SDP_TEXT_STR16:
    case SDP_TEXT_STR32:
    {
        xmlOutput.append("<text ");
        QByteArray text = QByteArray::fromRawData(data->val.str, data->unitSize);

        bool hasNonPrintableChar = false;
        for (int i = 0; i < text.count() && !hasNonPrintableChar; i++) {
            if (!isprint(text[i])) {
                hasNonPrintableChar = true;
                break;
            }
        }

        if (hasNonPrintableChar) {
            xmlOutput.append("encoding=\"hex\" value=\"");
            xmlOutput.append(text.toHex());
        } else {
            text.replace("&", "&amp");
            text.replace("<", "&lt");
            text.replace(">", "&gt");
            text.replace("\"", "&quot");

            xmlOutput.append("value=\"");
            xmlOutput.append(text);
        }

        xmlOutput.append("\"/>\n");
        break;
    }
    case SDP_BOOL:
        if (data->val.uint8)
            xmlOutput.append("<boolean value=\"true\"/>\n");
        else
            xmlOutput.append("<boolean value=\"false\"/>\n");
        break;
    case SDP_SEQ_UNSPEC:
        break;
    case SDP_SEQ8:
    case SDP_SEQ16:
    case SDP_SEQ32:
        xmlOutput.append("<sequence>\n");
        parseAttributeValues(data->val.dataseq, indentation + 1, xmlOutput);
        xmlOutput.append(indentString);
        xmlOutput.append("</sequence>\n");
        break;
    case SDP_ALT_UNSPEC:
        break;
    case SDP_ALT8:
    case SDP_ALT16:
    case SDP_ALT32:
        xmlOutput.append("<alternate>\n");
        parseAttributeValues(data->val.dataseq, indentation + 1, xmlOutput);
        xmlOutput.append(indentString);
        xmlOutput.append("</alternate>\n");
        break;
    case SDP_URL_STR_UNSPEC:
        break;
    case SDP_URL_STR8:
    case SDP_URL_STR16:
    case SDP_URL_STR32:
        strncpy(snBuffer, data->val.str, data->unitSize - 1);
        xmlOutput.append("<url value=\"");
        xmlOutput.append(snBuffer);
        xmlOutput.append("\"/>\n");
        break;
    default:
        qDebug(QT_BT_BLUEZ) << "Unknown dtd type";
    }

    parseAttributeValues(data->next, indentation, xmlOutput);
}

static void parseAttribute(void *value, void *extraData)
{
    sdp_data_t *data = (sdp_data_t *) value;
    QByteArray *xmlOutput = static_cast<QByteArray *>(extraData);

    char buffer[BUFFER_SIZE];

    ::qsnprintf(buffer, BUFFER_SIZE, "  <attribute id=\"0x%04x\">\n", data->attrId);
    xmlOutput->append(buffer);

    parseAttributeValues(data, 2, *xmlOutput);

    xmlOutput->append("  </attribute>\n");
}

// the resulting xml output is based on the already used xml parser
QByteArray parseSdpRecord(sdp_record_t *record)
{
    if (!record || !record->attrlist)
        return QByteArray();

    QByteArray xmlOutput;

    xmlOutput.append("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<record>\n");

    sdp_list_foreach(record->attrlist, parseAttribute, &xmlOutput);
    xmlOutput.append("</record>");

    return xmlOutput;
}

QT_END_NAMESPACE
