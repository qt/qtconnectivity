/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtNfc module of the Qt Toolkit.
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "textrecordeditor.h"
#include "urirecordeditor.h"
#include "mimeimagerecordeditor.h"
#include "smartposterrecordeditor.h"

#include <QtCore/QTime>
#include <QMenu>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QFileDialog>

#include <qnearfieldmanager.h>
#include <qnearfieldtarget.h>
#include <qndefrecord.h>
#include <qndefnfctextrecord.h>
#include <qndefnfcurirecord.h>
#include <qndefnfcsmartposterrecord.h>
#include <qndefmessage.h>

#include <QtCore/QDebug>

class EmptyRecordLabel : public RecordEditor
{
    Q_OBJECT

public:
    EmptyRecordLabel(QWidget *parent = 0) : RecordEditor(parent)
    {
        new QLabel(tr("Empty Record"), this);
    }
    ~EmptyRecordLabel() { }

    void setRecord(const QNdefRecord &record)
    {
        Q_UNUSED(record);
    }

    QNdefRecord record() const
    {
        return QNdefRecord();
    }
};

class UnknownRecordLabel : public RecordEditor
{
    Q_OBJECT

public:
    UnknownRecordLabel(QWidget *parent = 0) : RecordEditor(parent)
    {
        new QLabel(tr("Unknown Record Type"));
    }
    ~UnknownRecordLabel() { }

    void setRecord(const QNdefRecord &record) { m_record = record; }
    QNdefRecord record() const { return m_record; }

private:
    QNdefRecord m_record;
};

MainWindow::MainWindow(QWidget *parent)
:   QMainWindow(parent), ui(new Ui::MainWindow), m_manager(new QNearFieldManager(this)),
    m_touchAction(NoAction)
{
    ui->setupUi(this);

    ui->messageEditor->addAction(tr("NFC Text Record"), this, SLOT(addNfcTextRecord()));
    ui->messageEditor->addAction(tr("NFC URI Record"), this, SLOT(addNfcUriRecord()));
    ui->messageEditor->addAction(tr("MIME Image Record"), this, SLOT(addMimeImageRecord()));
    ui->messageEditor->addAction(tr("Smart Poster"), this, SLOT(addSmartPosterRecord()));
    ui->messageEditor->addAction(tr("Empty Record"), this, SLOT(addEmptyRecord()));
    ui->addRecord->setMenu(ui->messageEditor->actionMenu());

    connect(ui->clearMessage, SIGNAL(clicked()), ui->messageEditor, SLOT(clearRecords()));

    connect(m_manager, SIGNAL(targetDetected(QNearFieldTarget*)),
            this, SLOT(targetDetected(QNearFieldTarget*)));
    connect(m_manager, SIGNAL(targetLost(QNearFieldTarget*)),
            this, SLOT(targetLost(QNearFieldTarget*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addNfcTextRecord()
{
    ui->messageEditor->addRecordEditor(new TextRecordEditor);
}

void MainWindow::addNfcUriRecord()
{
    ui->messageEditor->addRecordEditor(new UriRecordEditor);
}

void MainWindow::addMimeImageRecord()
{
    ui->messageEditor->addRecordEditor(new MimeImageRecordEditor);
}

void MainWindow::addSmartPosterRecord()
{
    ui->messageEditor->addRecordEditor(new SmartPosterRecordEditor);
}

void MainWindow::addEmptyRecord()
{
    ui->messageEditor->addRecordEditor(new EmptyRecordLabel);
}

void MainWindow::loadMessage()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select NDEF Message"));
    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray ndef = file.readAll();

    ndefMessageRead(QNdefMessage::fromByteArray(ndef));

    file.close();
}

void MainWindow::saveMessage()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Select NDEF Message"));
    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(ui->messageEditor->ndefMessage().toByteArray());

    file.close();
}

void MainWindow::touchReceive()
{
    ui->status->setStyleSheet(QLatin1String("background: blue"));

    m_touchAction = ReadNdef;

    m_manager->setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess);
    m_manager->startTargetDetection();
}

void MainWindow::touchStore()
{
    ui->status->setStyleSheet(QLatin1String("background: yellow"));

    m_touchAction = WriteNdef;

    m_manager->setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    m_manager->startTargetDetection();
}

void MainWindow::targetDetected(QNearFieldTarget *target)
{
    switch (m_touchAction) {
    case NoAction:
        break;
    case ReadNdef:
        connect(target, SIGNAL(ndefMessageRead(QNdefMessage)),
                this, SLOT(ndefMessageRead(QNdefMessage)));
        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        m_request = target->readNdefMessages();
        break;
    case WriteNdef:
        connect(target, SIGNAL(ndefMessagesWritten()), this, SLOT(ndefMessageWritten()));
        connect(target, SIGNAL(error(QNearFieldTarget::Error,QNearFieldTarget::RequestId)),
                this, SLOT(targetError(QNearFieldTarget::Error,QNearFieldTarget::RequestId)));

        m_request = target->writeNdefMessages(QList<QNdefMessage>() << ui->messageEditor->ndefMessage());
        break;
    }
}

void MainWindow::targetLost(QNearFieldTarget *target)
{
    target->deleteLater();
}

void MainWindow::ndefMessageRead(const QNdefMessage &message)
{
    ui->messageEditor->clearRecords();

    foreach (const QNdefRecord &record, message) {
        if (record.isRecordType<QNdefNfcTextRecord>()) {
            ui->messageEditor->addRecordEditor(new TextRecordEditor, record);
        } else if (record.isRecordType<QNdefNfcUriRecord>()) {
            ui->messageEditor->addRecordEditor(new UriRecordEditor, record);
        } else if (record.isRecordType<QNdefNfcSmartPosterRecord>()) {
            ui->messageEditor->addRecordEditor(new SmartPosterRecordEditor, record);
        } else if (record.typeNameFormat() == QNdefRecord::Mime &&
                   record.type().startsWith("image/")) {
            ui->messageEditor->addRecordEditor(new MimeImageRecordEditor, record);
        } else if (record.typeNameFormat() == QNdefRecord::Empty) {
            ui->messageEditor->addRecordEditor(new EmptyRecordLabel);
        } else {
            ui->messageEditor->addRecordEditor(new UnknownRecordLabel, record);
        }
    }

    ui->status->setStyleSheet(QString());
    m_manager->setTargetAccessModes(QNearFieldManager::NoTargetAccess);
    m_manager->stopTargetDetection();
    m_request = QNearFieldTarget::RequestId();
    ui->statusBar->clearMessage();
}

void MainWindow::ndefMessageWritten()
{
    ui->status->setStyleSheet(QString());
    m_manager->setTargetAccessModes(QNearFieldManager::NoTargetAccess);
    m_manager->stopTargetDetection();
    m_request = QNearFieldTarget::RequestId();
    ui->statusBar->clearMessage();
}

void MainWindow::targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id)
{
    Q_UNUSED(error);
    Q_UNUSED(id);

    if (m_request == id) {
        switch (error) {
        case QNearFieldTarget::NoError:
            ui->statusBar->clearMessage();
            break;
        case QNearFieldTarget::UnsupportedError:
            ui->statusBar->showMessage(tr("Unsupported tag"));
            break;
        case QNearFieldTarget::TargetOutOfRangeError:
            ui->statusBar->showMessage(tr("Tag removed from field"));
            break;
        case QNearFieldTarget::NoResponseError:
            ui->statusBar->showMessage(tr("No response from tag"));
            break;
        case QNearFieldTarget::ChecksumMismatchError:
            ui->statusBar->showMessage(tr("Checksum mismatch"));
            break;
        case QNearFieldTarget::InvalidParametersError:
            ui->statusBar->showMessage(tr("Invalid parameters"));
            break;
        case QNearFieldTarget::NdefReadError:
            ui->statusBar->showMessage(tr("NDEF read error"));
            break;
        case QNearFieldTarget::NdefWriteError:
            ui->statusBar->showMessage(tr("NDEF write error"));
            break;
        default:
            ui->statusBar->showMessage(tr("Unknown error"));
        }

        ui->status->setStyleSheet(QString());
        m_manager->setTargetAccessModes(QNearFieldManager::NoTargetAccess);
        m_manager->stopTargetDetection();
        m_request = QNearFieldTarget::RequestId();
    }
}

#include "mainwindow.moc"
