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

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlExtensionPlugin>

#include "qqmlndefrecord.h"
#include "qdeclarativenearfield_p.h"
#include "qdeclarativendeffilter_p.h"
#include "qdeclarativendeftextrecord_p.h"
#include "qdeclarativendefurirecord_p.h"
#include "qdeclarativendefmimerecord_p.h"

QT_USE_NAMESPACE

class QNfcQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QNfcQmlPlugin(QObject *parent = 0) : QQmlExtensionPlugin(parent) { }
    void registerTypes(const char *uri)
    {
        Q_ASSERT(uri == QStringLiteral("QtNfc"));

        // @uri QtNfc

        // Register the 5.0 types
        int major = 5;
        int minor = 0;

        qmlRegisterType<QDeclarativeNearField>(uri, major, minor, "NearField");
        qmlRegisterType<QDeclarativeNdefFilter>(uri, major, minor, "NdefFilter");
        qmlRegisterType<QQmlNdefRecord>(uri, major, minor, "NdefRecord");
        qmlRegisterType<QDeclarativeNdefTextRecord>(uri, major, minor, "NdefTextRecord");
        qmlRegisterType<QDeclarativeNdefUriRecord>(uri, major, minor, "NdefUriRecord");
        qmlRegisterType<QDeclarativeNdefMimeRecord>(uri, major, minor, "NdefMimeRecord");

        // Register the 5.2 types
        minor = 2;
        qmlRegisterType<QDeclarativeNearField>(uri, major, minor, "NearField");
        qmlRegisterType<QDeclarativeNdefFilter>(uri, major, minor, "NdefFilter");
        qmlRegisterType<QQmlNdefRecord>(uri, major, minor, "NdefRecord");
        qmlRegisterType<QDeclarativeNdefTextRecord>(uri, major, minor, "NdefTextRecord");
        qmlRegisterType<QDeclarativeNdefUriRecord>(uri, major, minor, "NdefUriRecord");
        qmlRegisterType<QDeclarativeNdefMimeRecord>(uri, major, minor, "NdefMimeRecord");

        // Register the 5.4 types
        // introduces 5.4 version, other existing 5.2 exports become automatically available under 5.2-5.4l
        minor = 4;
        qmlRegisterType<QDeclarativeNearField>(uri, major, minor, "NearField");

        // Register the 5.5 types
        minor = 5;
        qmlRegisterType<QDeclarativeNearField, 1>(uri, major, minor, "NearField");


        // Register the latest Qt version as QML type version
        qmlRegisterModule(uri, QT_VERSION_MAJOR, QT_VERSION_MINOR);
    }
};

#include "plugin.moc"

