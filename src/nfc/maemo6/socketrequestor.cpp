/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "socketrequestor_p.h"

#include <QtCore/QMutex>
#include <QtCore/QHash>
#include <QtCore/QSocketNotifier>
#include <QtCore/QStringList>
#include <QtCore/QElapsedTimer>
#include <QtCore/QCoreApplication>
#include <QtDBus/QDBusObjectPath>

#include <dbus/dbus.h>

#include <sys/select.h>
#include <errno.h>

struct WatchNotifier
{
    DBusWatch *watch;
    QSocketNotifier *readNotifier;
    QSocketNotifier *writeNotifier;
};

static QVariant getVariantFromDBusMessage(DBusMessageIter *iter)
{
    dbus_bool_t bool_data;
    dbus_int32_t int32_data;
    dbus_uint32_t uint32_data;
    dbus_int64_t int64_data;
    dbus_uint64_t uint64_data;
    char *str_data;
    char char_data;
    int argtype = dbus_message_iter_get_arg_type(iter);

    switch (argtype) {
    case DBUS_TYPE_BOOLEAN: {
        dbus_message_iter_get_basic(iter, &bool_data);
        QVariant variant((bool)bool_data);
        return variant;
    }
    case DBUS_TYPE_ARRAY: {
        // Handle all arrays here
        int elem_type = dbus_message_iter_get_element_type(iter);
        DBusMessageIter array_iter;

        dbus_message_iter_recurse(iter, &array_iter);

        if (elem_type == DBUS_TYPE_BYTE) {
            QByteArray byte_array;
            do {
                dbus_message_iter_get_basic(&array_iter, &char_data);
                byte_array.append(char_data);
            } while (dbus_message_iter_next(&array_iter));
            QVariant variant(byte_array);
            return variant;
        } else if (elem_type == DBUS_TYPE_STRING) {
            QStringList str_list;
            do {
                dbus_message_iter_get_basic(&array_iter, &str_data);
                str_list.append(str_data);
            } while (dbus_message_iter_next(&array_iter));
            QVariant variant(str_list);
            return variant;
        } else {
            QVariantList variantList;
            do {
                variantList << getVariantFromDBusMessage(&array_iter);
            } while (dbus_message_iter_next(&array_iter));
            QVariant variant(variantList);
            return variant;
        }
        break;
    }
    case DBUS_TYPE_BYTE: {
        dbus_message_iter_get_basic(iter, &char_data);
        QChar ch(char_data);
        QVariant variant(ch);
        return variant;
    }
    case DBUS_TYPE_INT32: {
        dbus_message_iter_get_basic(iter, &int32_data);
        QVariant variant((int)int32_data);
        return variant;
    }
    case DBUS_TYPE_UINT32: {
        dbus_message_iter_get_basic(iter, &uint32_data);
        QVariant variant((uint)uint32_data);
        return variant;
    }
    case DBUS_TYPE_OBJECT_PATH:
    case DBUS_TYPE_STRING: {
        dbus_message_iter_get_basic(iter, &str_data);
        QString str(str_data);
        QVariant variant(str);
        return variant;
    }
    case DBUS_TYPE_INT64: {
        dbus_message_iter_get_basic(iter, &int64_data);
        QVariant variant((qlonglong)int64_data);
        return variant;
    }
    case DBUS_TYPE_UINT64: {
        dbus_message_iter_get_basic(iter, &uint64_data);
        QVariant variant((qulonglong)uint64_data);
        return variant;
    }
    case DBUS_TYPE_DICT_ENTRY:
    case DBUS_TYPE_STRUCT: {
        // Handle all structs here
        DBusMessageIter struct_iter;
        dbus_message_iter_recurse(iter, &struct_iter);

        QVariantList variantList;
        do {
            variantList << getVariantFromDBusMessage(&struct_iter);
        } while (dbus_message_iter_next(&struct_iter));
        QVariant variant(variantList);
        return variant;
    }
    case DBUS_TYPE_VARIANT: {
        DBusMessageIter variant_iter;
        dbus_message_iter_recurse(iter, &variant_iter);

        return getVariantFromDBusMessage(&variant_iter);
    }
    case DBUS_TYPE_UNIX_FD: {
        dbus_message_iter_get_basic(iter, &uint32_data);
        QVariant variant((uint)uint32_data);
        return variant;
    }

    default:
        qWarning("Unsupported DBUS type: %d\n", argtype);
    }

    return QVariant();
}

class SocketRequestorPrivate : public QObject
{
    Q_OBJECT

public:
    SocketRequestorPrivate();
    ~SocketRequestorPrivate();

    DBusHandlerResult messageFilter(DBusConnection *connection, DBusMessage *message);
    void addWatch(DBusWatch *watch);

    void registerObject(const QString &path, SocketRequestor *object);
    void unregisterObject(const QString &path);

    Q_INVOKABLE void sendRequestAccess(const QString &adaptor, const QString &path,
                                       const QString &kind);
    Q_INVOKABLE void sendCancelAccessRequest(const QString &adaptor, const QString &path,
                                             const QString &kind);

    bool waitForDBusSignal(int msecs);

private:
    bool parseAccessFailed(DBusMessage *message, SocketRequestor *socketRequestor);
    bool parseAccessGranted(DBusMessage *message, SocketRequestor *socketRequestor);
    bool parseAcceptConnect(DBusMessage *message, SocketRequestor *socketRequestor,
                            const char *member);
    bool parseSocket(DBusMessage *message, SocketRequestor *socketRequestor, const char *member);
    bool parseErrorDenied(DBusMessage *message, SocketRequestor *socketRequestor);

private slots:
    void socketRead();
    void socketWrite();

private:
    QMutex m_mutex;
    DBusConnection *m_dbusConnection;
    QHash<QString, SocketRequestor *> m_dbusObjects;
    QMap<quint32, SocketRequestor *> m_pendingCalls;
    QList<WatchNotifier> m_watchNotifiers;
};

Q_GLOBAL_STATIC(SocketRequestorPrivate, socketRequestorPrivate)

static DBusHandlerResult dbusFilter(DBusConnection *connection, DBusMessage *message,
                                    void *userData)
{
    SocketRequestorPrivate *s = static_cast<SocketRequestorPrivate *>(userData);
    return s->messageFilter(connection, message);
}

static dbus_bool_t dbusWatch(DBusWatch *watch, void *data)
{
    SocketRequestorPrivate *s = static_cast<SocketRequestorPrivate *>(data);
    s->addWatch(watch);

    return true;
}

SocketRequestorPrivate::SocketRequestorPrivate()
{
    DBusError error;
    dbus_error_init(&error);
    m_dbusConnection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
    dbus_connection_add_filter(m_dbusConnection, &dbusFilter, this, 0);
    dbus_connection_set_watch_functions(m_dbusConnection, dbusWatch, 0, 0, this, 0);
    dbus_error_free(&error);
}

SocketRequestorPrivate::~SocketRequestorPrivate()
{
    dbus_connection_close(m_dbusConnection);
    dbus_connection_unref(m_dbusConnection);
}

DBusHandlerResult SocketRequestorPrivate::messageFilter(DBusConnection *connection,
                                                        DBusMessage *message)
{
    QMutexLocker locker(&m_mutex);

    foreach (const WatchNotifier &watchNotifier, m_watchNotifiers)
        watchNotifier.writeNotifier->setEnabled(true);

    if (connection != m_dbusConnection)
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    SocketRequestor *socketRequestor;
    const QString path = QString::fromUtf8(dbus_message_get_path(message));
    quint32 serial = dbus_message_get_reply_serial(message);
    if (!path.isEmpty() && serial == 0)
        socketRequestor = m_dbusObjects.value(path);
    else if (path.isEmpty() && serial != 0)
        socketRequestor = m_pendingCalls.take(serial);
    else
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    enum {
        NotHandled,
        Handled,
        HandledSendReply
    } handled;

    if (dbus_message_is_method_call(message, "com.nokia.nfc.AccessRequestor", "AccessFailed"))
        handled = parseAccessFailed(message, socketRequestor) ? HandledSendReply : NotHandled;
    else if (dbus_message_is_method_call(message, "com.nokia.nfc.AccessRequestor", "AccessGranted"))
        handled = parseAccessGranted(message, socketRequestor) ? HandledSendReply : NotHandled;
    else if (dbus_message_is_method_call(message, "com.nokia.nfc.LLCPRequestor", "Accept"))
        handled = parseAcceptConnect(message, socketRequestor, "accept") ? HandledSendReply : NotHandled;
    else if (dbus_message_is_method_call(message, "com.nokia.nfc.LLCPRequestor", "Connect"))
        handled = parseAcceptConnect(message, socketRequestor, "connect") ? HandledSendReply : NotHandled;
    else if (dbus_message_is_method_call(message, "com.nokia.nfc.LLCPRequestor", "Socket"))
        handled = parseSocket(message, socketRequestor, "socket") ? HandledSendReply : NotHandled;
    else if (dbus_message_is_error(message, "com.nokia.nfc.Error.Denied"))
        handled = parseErrorDenied(message, socketRequestor) ? Handled : NotHandled;
    else
        handled = NotHandled;

    if (handled == NotHandled)
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    if (handled == HandledSendReply) {
        DBusMessage *reply = dbus_message_new_method_return(message);
        quint32 serial;
        dbus_connection_send(connection, reply, &serial);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

bool SocketRequestorPrivate::parseErrorDenied(DBusMessage *message,
                                              SocketRequestor *socketRequestor)
{
    Q_UNUSED(message);

    QMetaObject::invokeMethod(socketRequestor, "accessFailed",
                              Q_ARG(QDBusObjectPath, QDBusObjectPath()),
                              Q_ARG(QString, QLatin1String("")),
                              Q_ARG(QString, QLatin1String("Access denied")));
    return true;
}

bool SocketRequestorPrivate::parseAccessFailed(DBusMessage *message,
                                               SocketRequestor *socketRequestor)
{
    Q_UNUSED(message);

    // m_mutex is locked in messageFilter()

    DBusMessageIter args;

    if (!dbus_message_iter_init(message, &args))
        return false;

    // read DBus Object Path
    QVariant objectPath = getVariantFromDBusMessage(&args);

    if (!dbus_message_iter_next(&args))
        return false;

    // read DBus kind string
    QVariant kind = getVariantFromDBusMessage(&args);

    if (!dbus_message_iter_next(&args))
        return false;

    // read DBus error string
    QVariant errorString = getVariantFromDBusMessage(&args);

    QMetaObject::invokeMethod(socketRequestor, "accessFailed",
                              Q_ARG(QDBusObjectPath, QDBusObjectPath(objectPath.toString())),
                              Q_ARG(QString, kind.toString()),
                              Q_ARG(QString, errorString.toString()));
    return true;
}

bool SocketRequestorPrivate::parseAccessGranted(DBusMessage *message,
                                                SocketRequestor *socketRequestor)
{
    Q_UNUSED(message);

    // m_mutex is locked in messageFilter()

    DBusMessageIter args;

    if (!dbus_message_iter_init(message, &args))
        return false;

    // read DBus Object Path
    QVariant objectPath = getVariantFromDBusMessage(&args);

    if (!dbus_message_iter_next(&args))
        return false;

    // read access kind
    QVariant kind = getVariantFromDBusMessage(&args);

    QMetaObject::invokeMethod(socketRequestor, "accessGranted",
                              Q_ARG(QDBusObjectPath, QDBusObjectPath(objectPath.toString())),
                              Q_ARG(QString, kind.toString()));
    return true;
}

bool SocketRequestorPrivate::parseAcceptConnect(DBusMessage *message,
                                                SocketRequestor *socketRequestor,
                                                const char *member)
{
    // m_mutex is locked in messageFilter()

    DBusMessageIter args;

    if (!dbus_message_iter_init(message, &args))
        return false;

    // read DBus Variant (lsap)
    QVariant lsap = getVariantFromDBusMessage(&args);

    if (!dbus_message_iter_next(&args))
        return false;
    // read DBus Variant (rsap)
    QVariant rsap = getVariantFromDBusMessage(&args);

    if (!dbus_message_iter_next(&args))
        return false;

    // read socket fd
    QVariant fd = getVariantFromDBusMessage(&args);

    if (!dbus_message_iter_next(&args))
        return false;

    // read DBus a{sv} into QVariantMap
    QVariant prop = getVariantFromDBusMessage(&args);
    QVariantMap properties;
    foreach (const QVariant &v, prop.toList()) {
        QVariantList vl = v.toList();
        if (vl.length() != 2)
            continue;

        properties.insert(vl.first().toString(), vl.at(1));
    }

    QMetaObject::invokeMethod(socketRequestor, member, Q_ARG(QDBusVariant, QDBusVariant(lsap)),
                              Q_ARG(QDBusVariant, QDBusVariant(rsap)),
                              Q_ARG(int, fd.toInt()), Q_ARG(QVariantMap, properties));

    return true;
}

bool SocketRequestorPrivate::parseSocket(DBusMessage *message, SocketRequestor *socketRequestor,
                                         const char *member)
{
    // m_mutex is locked in messageFilter()

    DBusMessageIter args;

    if (!dbus_message_iter_init(message, &args))
        return false;

    // read DBus Variant (lsap)
    QVariant lsap = getVariantFromDBusMessage(&args);

    if (!dbus_message_iter_next(&args))
        return false;

    // read socket fd
    QVariant fd = getVariantFromDBusMessage(&args);

    if (!dbus_message_iter_next(&args))
        return false;

    // read DBus a{sv} into QVariantMap
    QVariant prop = getVariantFromDBusMessage(&args);
    QVariantMap properties;
    foreach (const QVariant &v, prop.toList()) {
        QVariantList vl = v.toList();
        if (vl.length() != 2)
            continue;

        properties.insert(vl.first().toString(), vl.at(1));
    }

    QMetaObject::invokeMethod(socketRequestor, member, Q_ARG(QDBusVariant, QDBusVariant(lsap)),
                              Q_ARG(int, fd.toInt()), Q_ARG(QVariantMap, properties));

    return true;
}

void SocketRequestorPrivate::addWatch(DBusWatch *watch)
{
    QMutexLocker locker(&m_mutex);

    int fd = dbus_watch_get_unix_fd(watch);

    WatchNotifier watchNotifier;
    watchNotifier.watch = watch;

    watchNotifier.readNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(watchNotifier.readNotifier, SIGNAL(activated(int)), this, SLOT(socketRead()));
    watchNotifier.writeNotifier = new QSocketNotifier(fd, QSocketNotifier::Write, this);
    connect(watchNotifier.writeNotifier, SIGNAL(activated(int)), this, SLOT(socketWrite()));

    m_watchNotifiers.append(watchNotifier);
}

void SocketRequestorPrivate::socketRead()
{
    QMutexLocker locker(&m_mutex);

    QList<DBusWatch *> pendingWatches;

    foreach (const WatchNotifier &watchNotifier, m_watchNotifiers) {
        if (watchNotifier.readNotifier != sender())
            continue;

        pendingWatches.append(watchNotifier.watch);
    }

    DBusConnection *connection = m_dbusConnection;
    locker.unlock();

    foreach (DBusWatch *watch, pendingWatches)
        dbus_watch_handle(watch, DBUS_WATCH_READABLE);

    while (dbus_connection_dispatch(connection) == DBUS_DISPATCH_DATA_REMAINS);
}

void SocketRequestorPrivate::socketWrite()
{
    QMutexLocker locker(&m_mutex);

    QList<DBusWatch *> pendingWatches;

    foreach (const WatchNotifier &watchNotifier, m_watchNotifiers) {
        if (watchNotifier.writeNotifier != sender())
            continue;

        watchNotifier.writeNotifier->setEnabled(false);
        pendingWatches.append(watchNotifier.watch);
    }

    locker.unlock();

    foreach (DBusWatch *watch, pendingWatches)
        dbus_watch_handle(watch, DBUS_WATCH_WRITABLE);
}

void SocketRequestorPrivate::registerObject(const QString &path, SocketRequestor *object)
{
    QMutexLocker locker(&m_mutex);

    m_dbusObjects.insert(path, object);
}

void SocketRequestorPrivate::unregisterObject(const QString &path)
{
    QMutexLocker locker(&m_mutex);

    m_dbusObjects.remove(path);
}

void SocketRequestorPrivate::sendRequestAccess(const QString &adaptor, const QString &path,
                                               const QString &kind)
{
    QMutexLocker locker(&m_mutex);

    foreach (const WatchNotifier &watchNotifier, m_watchNotifiers)
        watchNotifier.writeNotifier->setEnabled(true);

    DBusMessage *message;
    DBusMessageIter args;

    message = dbus_message_new_method_call("com.nokia.nfc", adaptor.toLocal8Bit(),
                                           "com.nokia.nfc.Adapter", "RequestAccess");

    if (!message)
        return;

    dbus_message_iter_init_append(message, &args);
    const QByteArray p = path.toUtf8();
    const char *pData = p.constData();
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_OBJECT_PATH, &pData)) {
        dbus_message_unref(message);
        return;
    }

    const QByteArray k = kind.toUtf8();
    const char *kData = k.constData();
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &kData)) {
        dbus_message_unref(message);
        return;
    }

    quint32 serial;
    dbus_connection_send(m_dbusConnection, message, &serial);

    dbus_message_unref(message);

    m_pendingCalls.insert(serial, m_dbusObjects.value(path));
}

void SocketRequestorPrivate::sendCancelAccessRequest(const QString &adaptor, const QString &path,
                                                     const QString &kind)
{
    QMutexLocker locker(&m_mutex);

    foreach (const WatchNotifier &watchNotifier, m_watchNotifiers)
        watchNotifier.writeNotifier->setEnabled(true);

    DBusMessage *message;
    DBusMessageIter args;

    message = dbus_message_new_method_call("com.nokia.nfc", adaptor.toLocal8Bit(),
                                           "com.nokia.nfc.Adapter", "CancelAccessRequest");

    if (!message)
        return;

    dbus_message_iter_init_append(message, &args);
    const QByteArray p = path.toUtf8();
    const char *pData = p.constData();
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_OBJECT_PATH, &pData)) {
        dbus_message_unref(message);
        return;
    }

    const QByteArray k = kind.toUtf8();
    const char *kData = k.constData();
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &kData)) {
        dbus_message_unref(message);
        return;
    }

    quint32 serial;
    dbus_connection_send(m_dbusConnection, message, &serial);

    dbus_message_unref(message);
}

bool SocketRequestorPrivate::waitForDBusSignal(int msecs)
{
    dbus_connection_flush(m_dbusConnection);

    fd_set rfds;
    FD_ZERO(&rfds);

    int nfds = -1;
    foreach (const WatchNotifier &watchNotifier, m_watchNotifiers) {
        FD_SET(watchNotifier.readNotifier->socket(), &rfds);
        nfds = qMax(nfds, watchNotifier.readNotifier->socket());
    }

    timeval timeout;
    timeout.tv_sec = msecs / 1000;
    timeout.tv_usec = (msecs % 1000) * 1000;

    // timeout can not be 0 or else select will return an error
    if (msecs == 0)
        timeout.tv_usec = 1000;

    int result = -1;
    // on Linux timeout will be updated by select, but _not_ on other systems
    result = ::select(nfds + 1, &rfds, 0, 0, &timeout);
    if (result == -1 && errno != EINTR)
        return false;

    foreach (const WatchNotifier &watchNotifier, m_watchNotifiers) {
        if (FD_ISSET(watchNotifier.readNotifier->socket(), &rfds)) {
            QMetaObject::invokeMethod(watchNotifier.readNotifier, "activated",
                                      Q_ARG(int, watchNotifier.readNotifier->socket()));
        }
    }

    return true;
}


SocketRequestor::SocketRequestor(const QString &adaptor, QObject *parent)
:   QObject(parent), m_adaptor(adaptor)
{


}

SocketRequestor::~SocketRequestor()
{
}

void SocketRequestor::requestAccess(const QString &path, const QString &kind)
{
    SocketRequestorPrivate *s = socketRequestorPrivate();

    s->registerObject(path, this);

    QMetaObject::invokeMethod(s, "sendRequestAccess", Qt::QueuedConnection,
                              Q_ARG(QString, m_adaptor), Q_ARG(QString, path),
                              Q_ARG(QString, kind));
}

void SocketRequestor::cancelAccessRequest(const QString &path, const QString &kind)
{
    SocketRequestorPrivate *s = socketRequestorPrivate();

    s->unregisterObject(path);

    QMetaObject::invokeMethod(s, "sendCancelAccessRequest", Qt::QueuedConnection,
                              Q_ARG(QString, m_adaptor), Q_ARG(QString, path),
                              Q_ARG(QString, kind));
}

bool SocketRequestor::waitForDBusSignal(int msecs)
{
    SocketRequestorPrivate *s = socketRequestorPrivate();

    // Send queued method calls, i.e. requestAccess() and cancelAccessRequest().
    QCoreApplication::sendPostedEvents(s, QEvent::MetaCall);

    // Wait for DBus signal.
    bool result = socketRequestorPrivate()->waitForDBusSignal(msecs);

    // Send queued method calls, i.e. those from DBus.
    QCoreApplication::sendPostedEvents(this, QEvent::MetaCall);

    return result;
}

#include <moc_socketrequestor_p.cpp>
#include <socketrequestor.moc>
