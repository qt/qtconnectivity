/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVENEARFIELD_P_H
#define QDECLARATIVENEARFIELD_P_H

#include <QtCore/QObject>
#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeParserStatus>

#include <qnearfieldmanager.h>
#include <qdeclarativendefrecord.h>

class QDeclarativeNdefFilter;

QTNFC_USE_NAMESPACE

class QDeclarativeNearField : public QObject, public QDeclarativeParserStatus
{
    Q_OBJECT

    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeNdefRecord> messageRecords READ messageRecords NOTIFY messageRecordsChanged)
    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeNdefFilter> filter READ filter NOTIFY filterChanged)
    Q_PROPERTY(bool orderMatch READ orderMatch WRITE setOrderMatch NOTIFY orderMatchChanged)

    Q_INTERFACES(QDeclarativeParserStatus)

public:
    explicit QDeclarativeNearField(QObject *parent = 0);

    QDeclarativeListProperty<QDeclarativeNdefRecord> messageRecords();

    QDeclarativeListProperty<QDeclarativeNdefFilter> filter();

    bool orderMatch() const;
    void setOrderMatch(bool on);

    // From QDeclarativeParserStatus
    void classBegin() { }
    void componentComplete();

signals:
    void messageRecordsChanged();
    void filterChanged();
    void orderMatchChanged();

private slots:
    void _q_handleNdefMessage(const QNdefMessage &message);

private:
    QList<QDeclarativeNdefRecord *> m_message;
    QList<QDeclarativeNdefFilter *> m_filter;
    bool m_orderMatch;
    bool m_componentCompleted;
    bool m_messageUpdating;

    QNearFieldManager *m_manager;
    int m_messageHandlerId;

    void registerMessageHandler();

    static void append_messageRecord(QDeclarativeListProperty<QDeclarativeNdefRecord> *list,
                                     QDeclarativeNdefRecord *record);
    static int count_messageRecords(QDeclarativeListProperty<QDeclarativeNdefRecord> *list);
    static QDeclarativeNdefRecord *at_messageRecord(QDeclarativeListProperty<QDeclarativeNdefRecord> *list,
                                                    int index);
    static void clear_messageRecords(QDeclarativeListProperty<QDeclarativeNdefRecord> *list);

    static void append_filter(QDeclarativeListProperty<QDeclarativeNdefFilter> *list,
                              QDeclarativeNdefFilter *filter);
    static int count_filters(QDeclarativeListProperty<QDeclarativeNdefFilter> *list);
    static QDeclarativeNdefFilter *at_filter(QDeclarativeListProperty<QDeclarativeNdefFilter> *list,
                                                    int index);
    static void clear_filter(QDeclarativeListProperty<QDeclarativeNdefFilter> *list);
};

#endif // QDECLARATIVENEARFIELD_P_H
