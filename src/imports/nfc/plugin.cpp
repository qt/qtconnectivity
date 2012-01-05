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


#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeExtensionPlugin>

#include <qdeclarativendefrecord.h>

#include "qdeclarativenearfieldsocket_p.h"
#include "qdeclarativenearfield_p.h"
#include "qdeclarativendeffilter_p.h"
#include "qdeclarativendeftextrecord_p.h"
#include "qdeclarativendefurirecord_p.h"
#include "qdeclarativendefmimerecord_p.h"

QT_USE_NAMESPACE

class QNfcQmlPlugin : public QDeclarativeExtensionPlugin
{
    Q_OBJECT
public:
    void registerTypes(const char *uri)
    {
        Q_ASSERT(uri == QLatin1String("QtNfc"));

        int major = 5;
        int minor = 0;
        qmlRegisterType<QDeclarativeNearFieldSocket>(uri, major, minor, "NearFieldSocket");

        qmlRegisterType<QDeclarativeNearField>(uri, major, minor, "NearField");
        qmlRegisterType<QDeclarativeNdefFilter>(uri, major, minor, "NdefFilter");
        qmlRegisterType<QDeclarativeNdefRecord>(uri, major, minor, "NdefRecord");
        qmlRegisterType<QDeclarativeNdefTextRecord>(uri, major, minor, "NdefTextRecord");
        qmlRegisterType<QDeclarativeNdefUriRecord>(uri, major, minor, "NdefUriRecord");
        qmlRegisterType<QDeclarativeNdefMimeRecord>(uri, major, minor, "NdefMimeRecord");
    }
};

#include "plugin.moc"

Q_EXPORT_PLUGIN2(qnfcqmlplugin, QNfcQmlPlugin);
