/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#include <QDebug>
#include <QQmlExtensionPlugin>

#include <QQmlEngine>
#include <qqml.h>
#include "qdeclarativebluetoothimageprovider_p.h"


QT_USE_NAMESPACE

// This is run in a low priority thread.
QImage BluetoothThumbnailImageProvider::requestImage(const QString &id, QSize *size, const QSize &req_size)
{
    if (m_thumbnails.contains(id)) {
        if (size)
            *size = req_size;
        return m_thumbnails.value(id).scaled(req_size);
    }

    /* url format:
        image://bluetoothicons/{hosttype}
     */


    QImage image(
            req_size.width() > 0 ? req_size.width() : 100,
            req_size.height() > 0 ? req_size.height() : 50,
            QImage::Format_RGB32);

    QString imageUrl;

    if (id == "default")
        imageUrl = QLatin1String(":/default.svg");

    imageUrl = imageUrl.isEmpty() ? QLatin1String(":/default.svg") : imageUrl;
    image.load(imageUrl);

    if (size)
        *size = image.size();

    m_thumbnails.insert(id, image);

    return image;
}

BluetoothThumbnailImageProvider::BluetoothThumbnailImageProvider()
    :QQuickImageProvider(QQuickImageProvider::Image)
{

}

BluetoothThumbnailImageProvider::~BluetoothThumbnailImageProvider()
{
}


