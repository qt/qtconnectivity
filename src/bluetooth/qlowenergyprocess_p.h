/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
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

#ifndef QLOWENERGYPROCESS_H
#define QLOWENERGYPROCESS_H

#include <QtCore>
#include <QObject>
#ifdef QT_QNX_BLUETOOTH
#include <QList>
#include <QPointer>
#include "qlowenergycontroller_p.h"
#endif
#ifdef QT_BLUEZ_BLUETOOTH
#include <QProcess>
#endif
QT_BEGIN_NAMESPACE

class QLowEnergyProcess: public QObject
{
    Q_OBJECT
    friend class QLowEnergyServiceInfoPrivate;
public:
    QLowEnergyProcess();
    ~QLowEnergyProcess();

    static QLowEnergyProcess *instance();
    bool isConnected() const;
#ifdef QT_QNX_BLUETOOTH
    static void handleEvent(const int, const char *, const char *);
    void addPointer(QLowEnergyControllerPrivate* classPointer);
#endif
#ifdef QT_BLUEZ_BLUETOOTH
    QProcess *getProcess();

    void startCommand(const QString &command);
    void executeCommand(const QString &command);
    void endProcess();
    void addConnection();

Q_SIGNALS:
    void replySend(const QString &reply);

private slots:
    void replyRead();
#endif

private:
#ifdef QT_BLUEZ_BLUETOOTH
    QProcess *m_process;
    int m_counter;
#endif
#ifdef QT_QNX_BLUETOOTH
    QList<QLowEnergyControllerPrivate*> m_classPointers;
#endif
    bool connected;
};
QT_END_NAMESPACE

#endif // QLOWENERGYPROCESS_H
