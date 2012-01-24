/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QAUTOMSGBOX_H
#define QAUTOMSGBOX_H

#include <QMessageBox>
#include <QTimer>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QCloseEvent>

class QAutoMsgBox : public QMessageBox
{
    Q_OBJECT

public:
    explicit QAutoMsgBox(QWidget *parent = 0);
    ~QAutoMsgBox();
    /*!
     * Call this method when you want to make the dialog dismissed
     * when the signalspy's count is equal to the \a count
    */
    void setSignalSpy(QSignalSpy* spy, int count)
    {
        if (spy == 0)
            return;
        m_spy = spy;
        m_signalCount = count;
        if (m_timer)
            delete m_timer;
        m_timer = new QTimer;
        m_timer->setInterval(50);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(timeOutFired()));
        m_timer->start();
    }
    /*!
     * Call this method when you want to make the dialog dismissed
     * after /a timeout(ms) period
    */
    void setDismissTimeOut(int timeout)
    {
        QTimer::singleShot(timeout, this, SLOT(singleShotSlot()));
    }
private slots:
    void timeOutFired()
    {
        if(m_spy->count() >= m_signalCount)
        {
            QApplication::postEvent(this, new QCloseEvent);
            m_timer->stop();
            delete m_timer;
            m_timer = 0;
        }
    }
    void singleShotSlot()
    {
        QApplication::postEvent(this, new QCloseEvent);
    }

private:
    bool event(QEvent *e);
    QTimer *m_timer;

    QSignalSpy *m_spy;//not own
    int m_signalCount;
};

#endif // QAUTOMSGBOX_H
