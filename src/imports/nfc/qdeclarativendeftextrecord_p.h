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

#ifndef QDECLARATIVENDEFTEXTRECORD_P_H
#define QDECLARATIVENDEFTEXTRECORD_P_H

#include <qqmlndefrecord.h>

#include <qndefnfctextrecord.h>

QT_USE_NAMESPACE

class QDeclarativeNdefTextRecord : public QQmlNdefRecord
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString locale READ locale WRITE setLocale NOTIFY localeChanged)
    Q_PROPERTY(LocaleMatch localeMatch READ localeMatch NOTIFY localeMatchChanged)

    Q_ENUMS(LocaleMatch)

public:
    enum LocaleMatch {
        LocaleMatchedNone,
        LocaleMatchedEnglish,
        LocaleMatchedLanguage,
        LocaleMatchedLanguageAndCountry
    };

    explicit QDeclarativeNdefTextRecord(QObject *parent = 0);
    Q_INVOKABLE QDeclarativeNdefTextRecord(const QNdefRecord &record, QObject *parent = 0);
    ~QDeclarativeNdefTextRecord();

    QString text() const;
    void setText(const QString &text);

    QString locale() const;
    void setLocale(const QString &locale);

    LocaleMatch localeMatch() const;

signals:
    void textChanged();
    void localeChanged();
    void localeMatchChanged();
};

#endif // QDECLARATIVENDEFTEXTRECORD_P_H
