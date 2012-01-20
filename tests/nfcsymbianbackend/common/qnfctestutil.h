/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Mobility Components.
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
#ifndef QNFCTESTUTIL_H
#define QNFCTESTUTIL_H
#include <QMessageBox>
#include "qautomsgbox.h"

const int MsgBoxTimeOutTime = 3*1000;
class QNfcTestUtil : public QObject
{
    Q_OBJECT
public:
    static void ShowMessage(const QString& message)
    {
        QMessageBox b(QMessageBox::Information, QObject::tr("NFC symbian backend test"),
        message, QMessageBox::Ok);
        b.exec();
    }
    static void ShowAutoMsg(const QString& message, QSignalSpy* spy, int count = 1)
    {
        QAutoMsgBox w;
        w.addButton(QMessageBox::Ok);
        w.setIcon(QMessageBox::Information);
        w.setText(message);
        w.setWindowTitle(QObject::tr("NFC symbian backend test"));
        w.setSignalSpy(spy, count);

        w.exec();
    }
    static void ShowAutoMsg(const QString& message, int mseconds = MsgBoxTimeOutTime)
    {
        QAutoMsgBox w;
        w.addButton(QMessageBox::Ok);
        w.setIcon(QMessageBox::Information);
        w.setText(message);
        w.setWindowTitle(QObject::tr("NFC symbian backend test"));
        w.setDismissTimeOut(mseconds);

        w.exec();
    }
};
#endif
