/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include "service.h"

#include <qbluetoothaddress.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothserviceinfo.h>


ServiceDiscoveryDialog::ServiceDiscoveryDialog(const QString &name,
                                               const QBluetoothAddress &address, QWidget *parent)
:   QDialog(parent), discoveryAgent(new QBluetoothServiceDiscoveryAgent(address)),
    ui(new Ui_ServiceDiscovery)
{
    ui->setupUi(this);

#if defined (Q_OS_SYMBIAN) || defined(Q_OS_WINCE) || defined(Q_WS_MAEMO_5) || defined(Q_WS_MAEMO_6)
    setWindowState(Qt::WindowMaximized);
#endif

    setWindowTitle(name);

    connect(discoveryAgent, SIGNAL(serviceDiscovered(const QBluetoothServiceInfo&)),
            this, SLOT(addService(const QBluetoothServiceInfo&)));
    connect(discoveryAgent, SIGNAL(finished()), ui->status, SLOT(hide()));

    discoveryAgent->start();
}

ServiceDiscoveryDialog::~ServiceDiscoveryDialog()
{
    delete discoveryAgent;
    delete ui;
}

void ServiceDiscoveryDialog::addService(const QBluetoothServiceInfo &info)
{
    if (info.serviceName().isEmpty())
        return;

    QString line = info.serviceName();
    if (!info.serviceDescription().isEmpty())
        line.append("\n\t" + info.serviceDescription());
    if (!info.serviceProvider().isEmpty())
        line.append("\n\t" + info.serviceProvider());

    ui->list->addItem(line);
}

