// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "textrecordeditor.h"
#include "ui_textrecordeditor.h"

TextRecordEditor::TextRecordEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TextRecordEditor)
{
    ui->setupUi(this);
}

TextRecordEditor::~TextRecordEditor()
{
    delete ui;
}

void TextRecordEditor::setRecord(const QNdefNfcTextRecord &textRecord)
{
    ui->text->setText(textRecord.text());
    ui->locale->setText(textRecord.locale());

    if (textRecord.encoding() == QNdefNfcTextRecord::Utf8)
        ui->encoding->setCurrentIndex(0);
    else if (textRecord.encoding() == QNdefNfcTextRecord::Utf16)
        ui->encoding->setCurrentIndex(1);
}

QNdefNfcTextRecord TextRecordEditor::record() const
{
    QNdefNfcTextRecord record;

    if (ui->encoding->currentIndex() == 0)
        record.setEncoding(QNdefNfcTextRecord::Utf8);
    else if (ui->encoding->currentIndex() == 1)
        record.setEncoding(QNdefNfcTextRecord::Utf16);

    record.setLocale(ui->locale->text());

    record.setText(ui->text->text());

    return record;
}
