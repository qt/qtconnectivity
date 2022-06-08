// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "urirecordeditor.h"
#include "ui_urirecordeditor.h"

#include <QtCore/QUrl>

UriRecordEditor::UriRecordEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UriRecordEditor)
{
    ui->setupUi(this);
}

UriRecordEditor::~UriRecordEditor()
{
    delete ui;
}

void UriRecordEditor::setRecord(const QNdefNfcUriRecord &uriRecord)
{
    ui->uri->setText(uriRecord.uri().toString());
}

QNdefNfcUriRecord UriRecordEditor::record() const
{
    QNdefNfcUriRecord record;

    record.setUri(ui->uri->text());

    return record;
}
