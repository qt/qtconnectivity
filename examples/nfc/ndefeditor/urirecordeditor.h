// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef URIRECORDEDITOR_H
#define URIRECORDEDITOR_H

#include <QtNfc/qndefnfcurirecord.h>

#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
    class UriRecordEditor;
}
QT_END_NAMESPACE

//! [0]
class UriRecordEditor : public QWidget
{
    Q_OBJECT

public:
    explicit UriRecordEditor(QWidget *parent = 0);
    ~UriRecordEditor();

    void setRecord(const QNdefNfcUriRecord &uriRecord);
    QNdefNfcUriRecord record() const;

private:
    Ui::UriRecordEditor *ui;
};
//! [0]
#endif // URIRECORDEDITOR_H
