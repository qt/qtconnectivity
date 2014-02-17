/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

#include <QtQml/QQmlEngine>
#include <QtQml/QQmlExtensionPlugin>

#include "qqmlndefrecord.h"
//#include "qdeclarativenearfieldsocket_p.h"
#include "qdeclarativenearfield_p.h"
#include "qdeclarativendeffilter_p.h"
#include "qdeclarativendeftextrecord_p.h"
#include "qdeclarativendefurirecord_p.h"
#include "qdeclarativendefmimerecord_p.h"

QT_USE_NAMESPACE

class QNfcQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri)
    {
        Q_ASSERT(uri == QStringLiteral("QtNfc"));

        // @uri QtNfc

        // Register the 5.0 types
        int major = 5;
        int minor = 0;
        //qmlRegisterType<QDeclarativeNearFieldSocket>(uri, major, minor, "NearFieldSocket");

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

        // Register the 5.3 types
        // introduces 5.3 version, other existing 5.2 exports automatically become availabe under 5.3 as well
        minor = 3;
        qmlRegisterType<QDeclarativeNearField>(uri, major, minor, "NearField");
    }
};

#include "plugin.moc"

