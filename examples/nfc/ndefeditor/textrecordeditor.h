// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TEXTRECORDEDITOR_H
#define TEXTRECORDEDITOR_H

#include <QtNfc/qndefnfctextrecord.h>

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
    class TextRecordEditor;
}
QT_END_NAMESPACE

//! [0]
class TextRecordEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TextRecordEditor(QWidget *parent = 0);
    ~TextRecordEditor();

    void setRecord(const QNdefNfcTextRecord &textRecord);
    QNdefNfcTextRecord record() const;

private:
    Ui::TextRecordEditor *ui;
};
//! [0]

#endif // TEXTRECORDEDITOR_H
