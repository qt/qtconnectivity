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

#ifndef QDECLARATIVENDEFURIRECORD_P_H
#define QDECLARATIVENDEFURIRECORD_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qqmlndefrecord.h>

#include <qndefnfcurirecord.h>

#include <QtCore/QMetaType>

QT_USE_NAMESPACE

class QDeclarativeNdefUriRecord : public QQmlNdefRecord
{
    Q_OBJECT

    Q_PROPERTY(QString uri READ uri WRITE setUri NOTIFY uriChanged)

public:
    explicit QDeclarativeNdefUriRecord(QObject *parent = 0);
    Q_INVOKABLE QDeclarativeNdefUriRecord(const QNdefRecord &record, QObject *parent = 0);
    ~QDeclarativeNdefUriRecord();

    QString uri() const;
    void setUri(const QString &uri);

signals:
    void uriChanged();
};

#endif // QDECLARATIVENDEFURIRECORD_P_H
