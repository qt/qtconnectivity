/****************************************************************************
**
** Copyright (C) 2013 Aaron McCarthy <mccarthy.aaron@gmail.com>
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

#include "smartposterrecordeditor.h"
#include "ui_smartposterrecordeditor.h"
#include "textrecordeditor.h"
#include "mimeimagerecordeditor.h"
#include "urirecordeditor.h"

#include <qndefnfcsmartposterrecord.h>
#include <qndefnfcsmartposter.h>

#include <QtCore/QDebug>

SmartPosterRecordEditor::SmartPosterRecordEditor(QWidget *parent)
:   RecordEditor(parent), ui(new Ui::SmartPosterRecordEditor)
{
    ui->setupUi(this);

    ui->spMessageEditor->addAction(tr("Title"), this, SLOT(addTitleRecord()));
    ui->spMessageEditor->addAction(tr("Icon"), this, SLOT(addIconRecord()));
    ui->add->setMenu(ui->spMessageEditor->actionMenu());

    ui->spMessageEditor->addRecordEditor(new UriRecordEditor);
}

SmartPosterRecordEditor::~SmartPosterRecordEditor()
{
    delete ui;
}

void SmartPosterRecordEditor::setRecord(const QNdefRecord &record)
{
    ui->spMessageEditor->clearRecords();

    if (!record.isRecordType<QNdefNfcSmartPosterRecord>()) {
        ui->spMessageEditor->addRecordEditor(new UriRecordEditor);
        return;
    }

    QNdefNfcSmartPosterRecord spRecord(record);

    foreach (const QNdefRecord &r, spRecord.smartPoster()) {
        qDebug() << r.typeNameFormat() << r.type();
        if (r.isRecordType<QNdefNfcSmartPoster::TitleRecord>())
            ui->spMessageEditor->addRecordEditor(new TextRecordEditor, r);
        else if (r.isRecordType<QNdefNfcSmartPoster::UriRecord>())
            ui->spMessageEditor->addRecordEditor(new UriRecordEditor, r);
        else if (r.isRecordType<QNdefNfcSmartPoster::ActionRecord>())
            continue;
        else if (r.isRecordType<QNdefNfcSmartPoster::SizeRecord>())
            continue;
        else if (r.isRecordType<QNdefNfcSmartPoster::TypeRecord>())
            continue;
        else if (r.typeNameFormat() == QNdefRecord::Mime && r.type().startsWith("image/"))
            ui->spMessageEditor->addRecordEditor(new MimeImageRecordEditor, r);
    }
}

QNdefRecord SmartPosterRecordEditor::record() const
{
    QNdefNfcSmartPosterRecord sp;
    sp.setSmartPoster(ui->spMessageEditor->ndefMessage());

    return sp;
}

void SmartPosterRecordEditor::addTitleRecord()
{
    ui->spMessageEditor->addRecordEditor(new TextRecordEditor);
}

void SmartPosterRecordEditor::addIconRecord()
{
    ui->spMessageEditor->addRecordEditor(new MimeImageRecordEditor);
}

void SmartPosterRecordEditor::on_clear_clicked()
{
    ui->spMessageEditor->clearRecords();
    ui->spMessageEditor->addRecordEditor(new UriRecordEditor);
}
