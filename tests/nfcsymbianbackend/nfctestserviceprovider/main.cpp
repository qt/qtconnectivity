/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "nfctestserviceprovider.h"

#include <QtGui>
#include <QApplication>
#include <qremoteserviceregister.h>
#include <qservicemanager.h>

#include <QDebug>
#include <qnearfieldmanager.h>
#include <qndefmessage.h>
#include <qnearfieldtarget.h>

#include <iostream>
#include <fstream>

using namespace std;

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

QTNFC_USE_NAMESPACE

void MyOutputHandler(QtMsgType type, const char *msg) {
    static int fd = -1;
      if (fd == -1)
          fd = ::open("E:\\nfctestserviceprovider.log", O_WRONLY | O_CREAT);

      ::write(fd, msg, strlen(msg));
      ::write(fd, "\n", 1);
      ::fsync(fd);

      switch (type) {
      case QtFatalMsg:
          abort();
      }
}

nfctestserviceprovider* w;

class MyContentHandler : public QObject
{
    Q_OBJECT

    
signals:
        void userHandleMessage(const QNdefMessage& msg, QNearFieldTarget* target);
        
public slots:
        void handleMessage(const QNdefMessage& msg, QNearFieldTarget* target)
        {
        QFile m_file("E:\\testserviceprovider.dat");
        m_file.open(QIODevice::ReadWrite | QIODevice::Append);
        QDataStream *m_dataStream = new QDataStream(&m_file);
        QByteArray msgArray = msg.toByteArray();
        (*m_dataStream) << msgArray;
        delete m_dataStream;
                       
        w->close();
        }
        
public:
    MyContentHandler(QObject* parent = 0)
        : QObject(parent)
    {
    qDebug() << " MyContentHandler constructed !!!!!" << endl;
    connect(this, SIGNAL(userHandleMessage(const QNdefMessage& , QNearFieldTarget* )),
            this, SLOT(handleMessage(const QNdefMessage& , QNearFieldTarget* )));
    }

};

void unregisterExampleService()
{
    QServiceManager m;
    m.removeService("nfctestserviceprovider");
}

void registerExampleService()
{
    unregisterExampleService();
    QServiceManager m;
    const QString path = QCoreApplication::applicationDirPath() + "/xmldata/nfctestserviceprovider.xml";
    qWarning() << "xml path:" << path;
    if (!m.addService(path))
        {
        qWarning() << "Cannot register service provider" << path;
        }
    else
        {
        qDebug() << " Register ok" << endl;
        }
}

Q_DECLARE_METATYPE(QMetaType::Type);



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    w = new nfctestserviceprovider();
    w->setWindowTitle( "nfc test service provider" );
    w->showMaximized();
    
    //qInstallMsgHandler(MyOutputHandler);
    
    qRegisterMetaType<QNearFieldTarget*>("QNearFieldTarget*"); 
    qRegisterMetaType<QNdefMessage>("QNdefMessage"); 

    registerExampleService();
    
    MyContentHandler handler;
    QNearFieldManager manager;
    
    int handle = manager.registerNdefMessageHandler(&handler, SIGNAL(userHandleMessage(QNdefMessage, QNearFieldTarget*)));
    int ret = app.exec();
    manager.unregisterNdefMessageHandler(handle);
    delete w;
    return ret;
}

#include "main.moc"
